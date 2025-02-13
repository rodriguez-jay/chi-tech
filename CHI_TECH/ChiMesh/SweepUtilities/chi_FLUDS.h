#ifndef _chi_FLUDS_h
#define _chi_FLUDS_h


#include <stdio.h>

#include "../MeshContinuum/chi_meshcontinuum.h"

typedef chi_mesh::CellSlab       TSlab;
typedef chi_mesh::CellPolygon    TPolygon;
typedef chi_mesh::CellPolyhedron TPolyhedron;

typedef int                      TVertexFace;
typedef int*                     TEdgeFace;
typedef chi_mesh::PolyFace       TPolyFace;

//face_slot index, vertex ids
typedef std::pair<int,std::vector<int>>             CompactFaceView;

//cell_global_id, faces
typedef std::pair<int,std::vector<CompactFaceView>> CompactCellView;

//###################################################################
/**Improved Flux Data Structure (FLUDS).*/
class chi_mesh::SweepManagement::FLUDS
{
public:
  int local_psi_stride;
  int local_psi_max_elements;



private:
  int largest_face;
  int G;
  int local_psi_Gn_block_stride;
  int local_psi_Gn_block_strideG;

  //======================================== References to psi vectors
  std::vector<double>*               ref_local_psi;
  std::vector<std::vector<double>>*  ref_deplocI_outgoing_psi;
  std::vector<std::vector<double>>*  ref_prelocI_outgoing_psi;
  std::vector<std::vector<double>>*  ref_boundryI_incoming_psi;

  //======================================== Alpha elements

  // This is a vector [cell_sweep_order_index][outgoing_face_count]
  // which holds the slot in the local multi-level psi vector
  std::vector<std::vector<int>>
    so_cell_outb_face_slot_indices;

  // This is a vector [cell_sweep_order_index][incoming_face_count]
  // that will holds a pair. Pair-first holds the slot where this
  // face's upwind data is stored. Pair-second is a mapping of
  // each of this face's dofs to the upwinded face's dofs
  std::vector<std::vector<std::pair<int,std::vector<int>> >>
    so_cell_inco_face_dof_indices;

public:
  // This is a small vector [deplocI] that holds the number of
  // face dofs for each dependent location.
  std::vector<int>    deplocI_face_dof_count;

  // Very small vector listing the boundaries this location depends on
  std::vector<int> boundary_dependencies;

private:
  // This is a vector [non_local_outgoing_face_count]
  // that maps a face to a dependent location and associated slot index
  std::vector<std::pair<int,int>>
    nonlocal_outb_face_deplocI_slot;

  // This is a vector [dependent_location][unordered_cell_index]
  // that holds an AlphaPair. AlphaPair-first is the cell's global_id
  // and AlphaPair-second holds a number of BetaPairs. Each BetaPair
  // comprises BetaPair-first = face_slot_index (the location of this
  // faces data in the psi vector, and then a vector of vertex indexes
  // that can be used for dof_mapping.
  std::vector<std::vector<CompactCellView>>
    deplocI_cell_views;



  //======================================== Beta elements
public:
  // This is a small vector [prelocI] that holds the number of
  // face dofs for each predecessor location.
  std::vector<int>    prelocI_face_dof_count;

private:
  // This is a vector [predecessor_location][unordered_cell_index]
  // that holds an AlphaPair. AlphaPair-first is the cell's global_id
  // and AlphaPair-second holds a number of BetaPairs. Each BetaPair
  // comprises BetaPair-first = face_slot_index (the location of this
  // faces data in the psi vector, and then a vector of vertex indexes
  // that can be used for dof_mapping.
  std::vector<std::vector<CompactCellView>>
    prelocI_cell_views;

  // This is a vector [nonlocal_inc_face_counter] containing
  // AlphaPairs. AlphaPair-first is the prelocI index and
  // AlphaPair-second is a BetaPair. The BetaPair-first is the slot where
  // the face storage begins and BetaPair-second is a dof mapping
  std::vector<std::pair<int,std::pair<int,std::vector<int>>>>
    nonlocal_inc_face_prelocI_slot_dof;


public:
  FLUDS(int in_G)
  {
    local_psi_stride=0;
    local_psi_max_elements=0;
    G=in_G;
  }


  void SetReferencePsi(std::vector<double>*               local_psi,
                       std::vector<std::vector<double>>*  deplocI_outgoing_psi,
                       std::vector<std::vector<double>>*  prelocI_outgoing_psi,
                       std::vector<std::vector<double>>*  boundryI_incoming_psi)
  {
    ref_local_psi = local_psi;
    ref_deplocI_outgoing_psi = deplocI_outgoing_psi;
    ref_prelocI_outgoing_psi = prelocI_outgoing_psi;
    ref_boundryI_incoming_psi = boundryI_incoming_psi;
  }

public:
  //01
  void InitializeAlphaElements(chi_mesh::SweepManagement::SPDS *spds);
  //02
  void InitializeBetaElements(chi_mesh::SweepManagement::SPDS *spds,
                              int tag_index=0);

  //chi_FLUDSv2.cc
  double*  OutgoingPsi(int cell_so_index, int outb_face_counter,
                       int face_dof, int n);
  double*  UpwindPsi(int cell_so_index, int inc_face_counter,
                     int face_dof,int g, int n);


  double*  NLOutgoingPsi(int outb_face_count,int face_dof, int n);

  double*  NLUpwindPsi(int nonl_inc_face_counter,
                       int face_dof,int g, int n);

  void AddFaceViewToDepLocI(int deplocI, int cell_g_index,
                            int face_slot, TVertexFace face_v_index);
  void AddFaceViewToDepLocI(int deplocI, int cell_g_index,
                            int face_slot, TEdgeFace edge_v_indices);
  void AddFaceViewToDepLocI(int deplocI, int cell_g_index,
                            int face_slot, TPolyFace* poly_face);


  void SerializeCellInfo(std::vector<CompactCellView>* cell_views,
                         std::vector<int>& face_indices,
                         int num_face_dofs);
  void DeSerializeCellInfo(std::vector<CompactCellView>& cell_views,
                           std::vector<int>* face_indices,
                           int& num_face_dofs);

  //01a
  void SlotDynamics(TSlab *slab_cell,
                    chi_mesh::SweepManagement::SPDS* spds,
                    std::vector<std::pair<int,short>>& lock_box,
                    std::set<int>& location_boundary_dependency_set);
  void IncidentMapping(TSlab *slab_cell,
                       chi_mesh::SweepManagement::SPDS* spds,
                       std::vector<int>&  local_so_cell_mapping);

  //01b
  void SlotDynamics(TPolygon *poly_cell,
                    chi_mesh::SweepManagement::SPDS* spds,
                    std::vector<std::pair<int,short>>& lock_box,
                    std::set<int>& location_boundary_dependency_set);
  void IncidentMapping(TPolygon *poly_cell,
                       chi_mesh::SweepManagement::SPDS* spds,
                       std::vector<int>&  local_so_cell_mapping);

  //01c polyhedron
  void SlotDynamics(TPolyhedron *polyh_cell,
                    chi_mesh::SweepManagement::SPDS* spds,
                    std::vector<std::pair<int,short>>& lock_box,
                    std::set<int>& location_boundary_dependency_set);
  void IncidentMapping(TPolyhedron *polyh_cell,
                       chi_mesh::SweepManagement::SPDS* spds,
                       std::vector<int>&  local_so_cell_mapping);

  //02a
  void NLIncidentMapping(TSlab *slab_cell,
                         chi_mesh::SweepManagement::SPDS* spds);
  //02b
  void NLIncidentMapping(TPolygon *poly_cell,
                         chi_mesh::SweepManagement::SPDS* spds);
  //02c
  void NLIncidentMapping(TPolyhedron *polyh_cell,
                         chi_mesh::SweepManagement::SPDS* spds);
};

#endif