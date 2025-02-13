#ifndef _cell_slab_h
#define _cell_slab_h

#include "cell.h"

//################################################################### Class def
/**Object to handle one dimensional slab cells.
 *
 * edges\n
 * [0] Neighbor cell index to the left.
 * [1] Neighbor cell index to the right.*/
class chi_mesh::CellSlab : public chi_mesh::Cell
{
public:
  int     v_indices[2];
  int     edges[2];
  Vector  face_normals[2];

public:
  CellSlab() : Cell(CellType::SLAB)
  {
    v_indices[0] = -1;
    v_indices[1] = -1;
    edges[0] = -1;
    edges[1] = -1;

    face_normals[0] = chi_mesh::Vector();
    face_normals[1] = chi_mesh::Vector();
  }
};

#endif
