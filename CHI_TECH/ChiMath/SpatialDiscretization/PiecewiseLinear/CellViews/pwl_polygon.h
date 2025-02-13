#ifndef _pwl_polygon_h
#define _pwl_polygon_h

#include "../pwl.h"
#include <vector>
#include "../../../../ChiMesh/Cell/cell_polygon.h"

/**For a given side(triangle), this structure holds the values of
 * shape functions at each quadrature point.*/
struct FEqp_data2d
{
  std::vector<double> shape_qp;
  std::vector<double> shape_qp_surf;
  std::vector<double> gradshapex_qp;
  std::vector<double> gradshapey_qp;
};
//Goes into
struct FEside_data2d
{
  double detJ;
  double detJ_surf;
  int*    v_index;
  chi_mesh::Matrix3x3 J;
  chi_mesh::Matrix3x3 Jinv;
  chi_mesh::Matrix3x3 JTinv;
  std::vector<FEqp_data2d*> qp_data;
};

//###################################################################
/** Object for handling polygon shaped 2D cells.
 *
 * This object has a whitepaper associated with it
 * (<a target="_blank"
 * href="../../whitepages/FEM/PWLPolygon/PWLPolygon.pdf">
 * here</a>)
 *
 * Notes on indexing:\n
 * - IntS_shapeI_shapeJ is indexed as [f][i][j]
 * - IntS_shapeI is indexed as [i][f]
 * - IntS_shapeI_gradshapeJ is indexed as [f][i][j]
 * - node_to_side_map is indexed as [i][f]
 * - edge_dof_mappings, is indexed as [f][fi] and
 *    returns cell dof i.*/
class PolygonFEView : public CellFEView
{
private:
  std::vector<FEside_data2d*> sides;
  chi_math::QuadratureTriangle* vol_quadrature;
  chi_math::QuadratureTriangle* surf_quadrature;
public:
  int      num_of_subtris;
  double   beta;
//  std::vector<chi_mesh::Vector> v01,v02;
  chi_mesh::Vertex vc;
  std::vector<double> detJ;
  std::vector<int*> node_to_side_map;
  std::vector<std::vector<int>> edge_dof_mappings;


//  std::vector<chi_math::QuadraturePointXY*> qpoints;
//  std::vector<double> w;

public:
  std::vector<double*>                          IntV_gradShapeI_gradShapeJ;
  std::vector<std::vector<chi_mesh::Vector>>    IntV_shapeI_gradshapeJ;
  std::vector<std::vector<double>>              IntV_shapeI_shapeJ;
  std::vector<double>                           IntV_shapeI;

  std::vector<chi_mesh::Vector>                 IntV_gradshapeI;
private:
  std::vector<std::vector<std::vector<double>>>           IntSi_shapeI_shapeJ;
  std::vector<std::vector<std::vector<chi_mesh::Vector>>> IntSi_shapeI_gradshapeJ;
public:
  std::vector<std::vector<std::vector<double>>> IntS_shapeI_shapeJ;
  std::vector<double*>                          IntS_shapeI;
  std::vector<std::vector<std::vector<chi_mesh::Vector>>> IntS_shapeI_gradshapeJ;

private:
  chi_mesh::MeshContinuum* grid;

  bool precomputed;
  
public:
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Constructor
  PolygonFEView(chi_mesh::CellPolygon* poly_cell,
                chi_mesh::MeshContinuum* vol_continuum,
                SpatialDiscretization_PWL *discretization);

  double Shape_xy(int i, chi_mesh::Vector xyz);
  chi_mesh::Vector GradShape_xy(int i, chi_mesh::Vector xyz);

  //############################################### Precomputation cell matrices
  double PreShape(int s, int i, int qpoint_index, bool on_surface = false);
  double PreGradShape_x(int s, int i, int qpoint_index);
  double PreGradShape_y(int s, int i, int qpoint_index);

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Determinant J
  double DetJ(int s, int qpoint_index, bool on_surface=false)
  {
    if (!on_surface)
      return sides[s]->detJ;
    else
      return sides[s]->detJ_surf;
  }

private:
  double GetShape(int side, int i, int qp, bool surface = false)
  {
    if (surface)
      return sides[side]->qp_data[i]->shape_qp_surf[qp];
    else
      return sides[side]->qp_data[i]->shape_qp[qp];
  }

  double GetGradShape_x(int side, int i, int qp)
  {
    return sides[side]->qp_data[i]->gradshapex_qp[qp];
  }

  double GetGradShape_y(int side, int i, int qp)
  {
    return sides[side]->qp_data[i]->gradshapey_qp[qp];
  }

public:
  void PreCompute();

};

#endif