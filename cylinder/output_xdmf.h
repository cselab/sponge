static int output_xdmf(double t, scalar *list, vector *vlist,
                       int cond(double, double, double, double),
                       const char *path) {
  float *xyz, *attr;
  long j, nattr, nvect, ncell, ncell_total, nsize, offset;
  char xyz_path[FILENAME_MAX], attr_path[FILENAME_MAX], xdmf_path[FILENAME_MAX],
      *vname, *xyz_base, *attr_base;
  FILE *file;
  MPI_File mpi_file;
  const int shift[8][3] = {
      {0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0},
      {1, 0, 0}, {1, 0, 1}, {1, 1, 1}, {1, 1, 0},
  };

  snprintf(xyz_path, sizeof xyz_path, "%s.xyz.raw", path);
  snprintf(attr_path, sizeof attr_path, "%s.attr.raw", path);
  snprintf(xdmf_path, sizeof xdmf_path, "%s.xdmf2", path);

  xyz_base = xyz_path;
  attr_base = attr_path;
  for (j = 0; xyz_path[j] != '\0'; j++) {
    if (xyz_path[j] == '/' && xyz_path[j + 1] != '\0') {
      xyz_base = &xyz_path[j + 1];
      attr_base = &attr_path[j + 1];
    }
  }

  nsize = 0;
  ncell = 0;
  j = 0;
  xyz = NULL;
  foreach_cell() if (is_local(cell) && is_leaf(cell) &&
                     (!cond || cond(x, y, z, Delta))) {
    long i;
    ncell++;
    if (ncell >= nsize) {
      nsize = 2 * nsize + 1;
      if ((xyz = realloc(xyz, 8 * 3 * nsize * sizeof *xyz)) == NULL) {
        fprintf(stderr, "%s:%d: realloc failed\n", __FILE__, __LINE__);
        return 1;
      }
    }
    for (i = 0; i < 8; i++) {
      xyz[j++] = x + Delta * (shift[i][0] - 0.5);
      xyz[j++] = y + Delta * (shift[i][1] - 0.5);
      xyz[j++] = z + Delta * (shift[i][2] - 0.5);
    }
  }

  if ((file = fopen(xyz_path, "w")) == NULL) {
    fprintf(stderr, "%s:%d: fail to open '%s'\n", __FILE__, __LINE__, xyz_path);
    return 1;
  }

  MPI_Exscan(&ncell, &offset, 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD);
  if (pid() == 0)
    offset = 0;
  MPI_File_open(MPI_COMM_WORLD, xyz_path, MPI_MODE_CREATE | MPI_MODE_WRONLY,
                MPI_INFO_NULL, &mpi_file);
  MPI_File_write_at_all(mpi_file, 3 * 8 * offset * sizeof *xyz, xyz,
                        3 * 8 * ncell * sizeof *xyz, MPI_BYTE,
                        MPI_STATUS_IGNORE);
  free(xyz);
  MPI_File_close(&mpi_file);

  nattr = list_len(list);
  nvect = vectors_len(vlist);
  if ((attr = malloc((nattr + 3 * nvect) * ncell * sizeof *attr)) == NULL) {
    fprintf(stderr, "%s:%d: malloc failed\n", __FILE__, __LINE__);
    return 1;
  }
  j = 0;
  foreach_cell() if (is_local(cell) && is_leaf(cell) &&
                     (!cond || cond(x, y, z, Delta))) {
    for (scalar s in list)
      attr[j++] = val(s);
    for (vector v in vlist) {
      attr[j++] = val(v.x);
      attr[j++] = val(v.y);
      attr[j++] = val(v.z);
    }
  }
  assert(j == (nattr + 3 * nvect) * ncell);
  MPI_File_open(MPI_COMM_WORLD, attr_path, MPI_MODE_CREATE | MPI_MODE_WRONLY,
                MPI_INFO_NULL, &mpi_file);
  MPI_File_write_at_all(mpi_file, (nattr + 3 * nvect) * offset * sizeof *attr,
                        attr, (nattr + 3 * nvect) * ncell * sizeof *attr,
                        MPI_BYTE, MPI_STATUS_IGNORE);
  free(attr);
  MPI_File_close(&mpi_file);

  if (pid() == npe() - 1) {
    ncell_total = offset + ncell;
    if ((file = fopen(xdmf_path, "w")) == NULL) {
      fprintf(stderr, "%s:%d: fail to open '%s'\n", __FILE__, __LINE__,
              xdmf_path);
      return 1;
    }
    fprintf(file,
            "<Xdmf\n"
            "    Version=\"2\">\n"
            "  <Domain>\n"
            "    <Grid>\n"
            "      <Time\n"
            "          Value=\"%.16e\"/>\n"
            "      <Topology\n"
            "          TopologyType=\"Hexahedron\"\n"
            "          Dimensions=\"%ld\"/>\n"
            "      <Geometry>\n"
            "        <DataItem\n"
            "            Dimensions=\"%ld 3\"\n"
            "            Format=\"Binary\">\n"
            "          %s\n"
            "        </DataItem>\n"
            "      </Geometry>\n",
            t, ncell_total, 8 * ncell_total, xyz_base);
    j = 0;
    for (scalar s in list)
      fprintf(file,
              "      <Attribute\n"
              "          Name=\"%s\"\n"
              "          Center=\"Cell\">\n"
              "        <DataItem\n"
              "            ItemType=\"HyperSlab\"\n"
              "            Dimensions=\"%ld\"\n"
              "            Type=\"HyperSlab\">\n"
              "          <DataItem Dimensions=\"3 1\">\n"
              "            %ld %ld %ld\n"
              "          </DataItem>\n"
              "          <DataItem\n"
              "              Dimensions=\"%ld\"\n"
              "              Format=\"Binary\">\n"
              "            %s\n"
              "          </DataItem>\n"
              "         </DataItem>\n"
              "      </Attribute>\n",
              s.name, ncell_total, j++, nattr + 3 * nvect, ncell_total,
              (nattr + 3 * nvect) * ncell_total, attr_base);
    for (vector v in vlist) {
      vname = strdup(v.x.name);
      *strrchr(vname, '.') = '\0';
      fprintf(file,
              "      <Attribute\n"
              "          Name=\"%s\"\n"
              "          AttributeType=\"Vector\"\n"
              "          Center=\"Cell\">\n"
              "        <DataItem\n"
              "            ItemType=\"HyperSlab\"\n"
              "            Dimensions=\"%ld 3\"\n"
              "            Type=\"HyperSlab\">\n"
              "          <DataItem Dimensions=\"3 2\">\n"
              "            0 %ld\n"
              "            1 1\n"
              "            %ld 3\n"
              "          </DataItem>\n"
              "          <DataItem\n"
              "              Dimensions=\"%ld %ld\"\n"
              "              Format=\"Binary\">\n"
              "            %s\n"
              "          </DataItem>\n"
              "         </DataItem>\n"
              "      </Attribute>\n",
              vname, ncell_total, j, ncell_total, ncell_total,
              nattr + 3 * nvect, attr_base);
      free(vname);
      j += 3;
    }
    fprintf(file, "    </Grid>\n"
                  "  </Domain>\n"
                  "</Xdmf>\n");
    if (fclose(file) != 0) {
      fprintf(stderr, "%s:%d: error: fail to close '%s'\n", __FILE__, __LINE__,
              xdmf_path);
      return 1;
    }
  }
  return 0;
}
