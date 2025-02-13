#include "chi_FLUDS.h"

#include "chi_SPDS.h"

#include "../../ChiMesh/Cell/cell_slab.h"

#include <chi_log.h>

extern ChiLog     chi_log;

//###################################################################
/**Performs non-local incident mapping for polyhedron cells.*/
void chi_mesh::SweepManagement::FLUDS::
  NLIncidentMapping(TSlab *slab_cell,
                    chi_mesh::SweepManagement::SPDS* spds)
{
  chi_mesh::MeshContinuum*         grid = spds->grid;
  chi_mesh::SweepManagement::SPLS* spls = spds->spls;

  //=================================================== Loop over faces
  //           INCIDENT                                 but process
  //                                                    only incident faces
  int num_faces = 2;
  for (short f=0; f<num_faces; f++)
  {
    double mu = slab_cell->face_normals[f].Dot(spds->omega);

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Incident face
    if (mu<0.0)
    {
      int neighbor = slab_cell->edges[f];

      if ((!grid->IsCellLocal(neighbor)) && (!grid->IsCellBndry(neighbor)))
      {
        //============================== Find prelocI
        int locJ = grid->cells[neighbor]->partition_id;
        int prelocI = spds->MapLocJToPrelocI(locJ);

        //============================== Find the cell in prelocI cell views
        int ass_cell = -1;
        for (int c=0; c<prelocI_cell_views[prelocI].size(); c++)
        {
          if (prelocI_cell_views[prelocI][c].first == neighbor)
          {
            ass_cell = c;
            break;
          }
        }
        if (ass_cell<0)
        {
          chi_log.Log(LOG_ALL)
            << "Required predecessor cell not located in call to"
            << " InitializeBetaElements. locJ=" << locJ
            << " prelocI=" << prelocI
            << " cell=" << neighbor;
          exit(EXIT_FAILURE);
        }

        //============================== Find associated face
        CompactCellView* adj_cell_view =
          &prelocI_cell_views[prelocI][ass_cell];
        int ass_face = -1;
        for (int af=0; af<adj_cell_view->second.size(); af++)
        {
          bool face_matches = true;
          for (int afv=0; afv<adj_cell_view->second[af].second.size(); afv++)
          {
            bool match_found = false;
            for (int fv=0; fv<1; fv++)
            {
              if (adj_cell_view->second[af].second[afv] ==
                  slab_cell->v_indices[f])
              {
                match_found = true;
                break;
              }
            }

            if (!match_found){face_matches = false; break;}
          }

          if (face_matches){ass_face = af; break;}
        }
        if (ass_face<0)
        {
          chi_log.Log(LOG_ALL)
            << "Associated face not found in call to InitializeBetaElements";
          exit(EXIT_FAILURE);
        }

        //============================== Map dofs
        std::pair<int,std::vector<int>> dof_mapping;
        dof_mapping.first = adj_cell_view->second[ass_face].first;
        std::vector<int>* ass_face_verts =
          &adj_cell_view->second[ass_face].second;
        for (int fv=0; fv<1; fv++)
        {
          bool match_found = false;
          for (int afv=0; afv<ass_face_verts->size(); afv++)
          {
            if (slab_cell->v_indices[f] ==
                ass_face_verts->operator[](afv))
            {
              match_found = true;
              dof_mapping.second.push_back(afv);
              break;
            }
          }

          if (!match_found)
          {
            chi_log.Log(LOG_ALL)
              << "Associated vertex not found in call to InitializeBetaElements";
            exit(EXIT_FAILURE);
          }
        }

        //============================== Push back final face info
        std::pair<int,std::pair<int,std::vector<int>>> inc_face_prelocI_info;
        inc_face_prelocI_info.first = prelocI;
        inc_face_prelocI_info.second =
          std::pair<int,std::vector<int>>(dof_mapping);

        nonlocal_inc_face_prelocI_slot_dof.push_back(inc_face_prelocI_info);


      }//if not local and not boundary
    }//if incident
  }//for incindent f


}