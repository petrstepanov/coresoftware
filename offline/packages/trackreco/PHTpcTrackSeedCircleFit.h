// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef PHTPCTRACKSEEDCIRCLEFIT_H
#define PHTPCTRACKSEEDCIRCLEFIT_H

#include <fun4all/SubsysReco.h>
#include <trackbase/ActsSurfaceMaps.h>
#include <trackbase/ActsTrackingGeometry.h>

#include <string>
#include <vector>

class PHCompositeNode;
class SvtxTrackMap;
class SvtxTrack;
class TrkrCluster;
class TF1;
class TrkrClusterContainer;

class PHTpcTrackSeedCircleFit : public SubsysReco
{
 public:

  PHTpcTrackSeedCircleFit(const std::string &name = "PHTpcTrackSeedCircleFit");

  ~PHTpcTrackSeedCircleFit() override;

 protected:
  int InitRun(PHCompositeNode* topNode) override;

  int process_event(PHCompositeNode*) override;

  int End(PHCompositeNode*) override;
  void use_truth_clusters(bool truth)
  { _use_truth_clusters = truth; }

  void set_track_map_name(const std::string &map_name) { _track_map_name = map_name; }
  void SetIteration(int iter){_n_iteration = iter;} 

 private:

  int GetNodes(PHCompositeNode* topNode);

  void  line_fit_clusters(std::vector<Acts::Vector3D>& globPos, double &a, double &b);
  void  line_fit(std::vector<std::pair<double,double>> points, double &a, double &b);
  void CircleFitByTaubin (std::vector<std::pair<double,double>> points, double &R, double &X0, double &Y0);
  std::vector<double> GetCircleClusterResiduals(std::vector<std::pair<double,double>> points, double R, double X0, double Y0);
  std::vector<double> GetLineClusterResiduals(std::vector<std::pair<double,double>> points, double A, double B);
  std::vector<TrkrCluster*> getTrackClusters(SvtxTrack *_tracklet_tpc);
  void findRoot(const double R, const double X0, const double Y0, double& x, double& y);
						    
  std::string _track_map_name_silicon;

  ActsSurfaceMaps *_surfmaps{nullptr};
  ActsTrackingGeometry *_tGeometry{nullptr};
  SvtxTrack *_tracklet_tpc{nullptr};
  SvtxTrackMap *_track_map{nullptr};

  unsigned int _min_tpc_layer = 7;
  unsigned int _max_tpc_layer = 54;

  double _z_proj= 0;
  
  bool _use_truth_clusters = false;
  TrkrClusterContainer *_cluster_map = nullptr;
  int _n_iteration = 0;
  std::string _track_map_name = "SvtxTrackMap";
};

#endif // PHTRACKSEEDVERTEXASSOCIATION_H
