#include "lbs_linear_boltzman_solver.h"
#include <ChiMesh/MeshHandler/chi_meshhandler.h>
#include <ChiMesh/VolumeMesher/chi_volumemesher.h>
#include <ChiMesh/VolumeMesher/Linemesh1D/volmesher_linemesh1d.h>
#include <ChiMesh/VolumeMesher/Extruder/volmesher_extruder.h>
#include <ChiMesh/VolumeMesher/Predefined2D/volmesher_predefined2d.h>


#include <chi_mpi.h>
#include <chi_log.h>

extern ChiMPI chi_mpi;
extern ChiLog chi_log;

typedef chi_mesh::SweepManagement::AngleSet TAngleSet;
typedef chi_mesh::SweepManagement::AngleSetGroup TAngleSetGroup;

#include <iomanip>
#include "ChiConsole/chi_console.h"

extern ChiConsole chi_console;

//###################################################################
/**Initializes the sweep ordering for the given groupset.*/
void LinearBoltzmanSolver::ComputeSweepOrderings(LBSGroupset *groupset)
{
  chi_log.Log(LOG_0) << "Computing Sweep ordering.\n";

  //============================================= Clear sweep ordering
  sweep_orderings.clear();
  sweep_orderings.shrink_to_fit();

  chi_mesh::MeshHandler*    mesh_handler = chi_mesh::GetCurrentHandler();
  chi_mesh::VolumeMesher*         mesher = mesh_handler->volume_mesher;

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 1D MESHES
  if (typeid(*mesher) == typeid(chi_mesh::VolumeMesherLinemesh1D))
  {
    int num_azi = groupset->quadrature->azimu_ang.size();
    int num_pol = groupset->quadrature->polar_ang.size();
    int pa      = num_pol/2;

    if (num_azi != 1)
    {
      chi_log.Log(LOG_0)
        << "Incompatible number of azimuthal angles in quadrature set "
        << "for a 1D simulation.";
      exit(EXIT_FAILURE);
    }

    chi_mesh::SweepManagement::SPDS* new_swp_order =
      chi_mesh::SweepManagement::
      CreateSweepOrder(groupset->quadrature->polar_ang[0],
                       groupset->quadrature->azimu_ang[0],
                       this->grid,
                       groupset->groups.size());
    this->sweep_orderings.push_back(new_swp_order);

    new_swp_order =
      chi_mesh::SweepManagement::
      CreateSweepOrder(groupset->quadrature->polar_ang[pa],
                       groupset->quadrature->azimu_ang[0],
                       this->grid,
                       groupset->groups.size());
    this->sweep_orderings.push_back(new_swp_order);
  }
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 2D MESHES
  else if (typeid(*mesher) == typeid(chi_mesh::VolumeMesherPredefined2D))
  {
    int num_azi = groupset->quadrature->azimu_ang.size();
    int num_pol = groupset->quadrature->polar_ang.size();

    if (num_pol != 1)
    {
      chi_log.Log(LOG_0)
        << "Incompatible number of polar angles in quadrature set "
        << "for a 2D simulation.";
      exit(EXIT_FAILURE);
    }

    for (int i=0; i<num_azi; i++)
    {
      chi_mesh::SweepManagement::SPDS* new_swp_order =
        chi_mesh::SweepManagement::
        CreateSweepOrder(groupset->quadrature->polar_ang[0],
                         groupset->quadrature->azimu_ang[i],
                         this->grid,
                         groupset->groups.size());
      this->sweep_orderings.push_back(new_swp_order);
    }
  }
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% EXTRUDED MESHES
  else if (typeid(*mesher) == typeid(chi_mesh::VolumeMesherExtruder))
  {
    int num_azi = groupset->quadrature->azimu_ang.size();
    int num_pol = groupset->quadrature->polar_ang.size();

    if (num_azi < 4)
    {
      chi_log.Log(LOG_0)
        << "Incompatible number of azimuthal angles in quadrature set "
        << "for a 3D simulation.";
      exit(EXIT_FAILURE);
    }
    if (num_pol < 2)
    {
      chi_log.Log(LOG_0)
        << "Incompatible number of polar angles in quadrature set "
        << "for a 3D simulation.";
      exit(EXIT_FAILURE);
    }

    //============================================= Create sweep ordering
    //                                              per azimuthal angle
    //                                              per hemisphere

    //=========================================== TOP HEMISPHERE
    for (int i=0; i<num_azi; i++)
    {
      chi_mesh::SweepManagement::SPDS* new_swp_order =
        chi_mesh::SweepManagement::
                  CreateSweepOrder(groupset->quadrature->polar_ang[0],
                                   groupset->quadrature->azimu_ang[i],
                                   this->grid,
                                   groupset->groups.size());
      this->sweep_orderings.push_back(new_swp_order);
    }
    //=========================================== BOTTOM HEMISPHERE
    int pa = num_pol/2;
    for (int i=0; i<num_azi; i++)
    {
      chi_mesh::SweepManagement::SPDS* new_swp_order =
        chi_mesh::SweepManagement::
        CreateSweepOrder(groupset->quadrature->polar_ang[pa],
                         groupset->quadrature->azimu_ang[i],
                         this->grid,
                         groupset->groups.size());
      this->sweep_orderings.push_back(new_swp_order);
    }

  }
  else
  {
    fprintf(stderr,"ERROR: Cannot create sweep ordering"
                   " for given mesh type.\n");
    exit(EXIT_FAILURE);
  }


  chi_log.Log(LOG_0)
    << "Done computing sweep orderings.           Process memory = "
    << std::setprecision(3)
    << chi_console.GetMemoryUsageInMB() << " MB";

}
