#ifndef TRACKRECO_PHTPCRESIDUALS_H
#define TRACKRECO_PHTPCRESIDUALS_H

#include <fun4all/SubsysReco.h>
#include <trackbase/TrkrDefs.h>

#include <trackbase/ActsTrackingGeometry.h>
#include <trackbase/ActsSurfaceMaps.h>

#include <Acts/Utilities/Definitions.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Utilities/Result.hpp>

#include <Acts/EventData/TrackParameters.hpp>
#include <ActsExamples/EventData/Track.hpp>

class PHCompositeNode;
class SvtxTrack;
class SvtxTrackMap;
class TpcSpaceChargeMatrixContainer;
class TrkrCluster;
class TrkrClusterContainer;

namespace ActsExamples
{
  class TrkrClusterSourceLink;
}

#include <memory>
class TFile;
class TH1;
class TH2;
class TTree;

using SourceLink = ActsExamples::TrkrClusterSourceLink;
using BoundTrackParamPtr = 
  std::unique_ptr<const Acts::BoundTrackParameters>;
using BoundTrackParamPtrResult = Acts::Result<BoundTrackParamPtr>;

/**
 * This class takes preliminary fits from PHActsTrkFitter to the 
 * silicon + MM clusters and calculates the residuals in the TPC 
 * from that track fit. The TPC state has to be explicitly determined
 * here since the Acts::DirectNavigator does not visit the TPC states
 */
class PHTpcResiduals : public SubsysReco
{

 public:

  PHTpcResiduals(const std::string &name = "PHTpcResiduals");
  ~PHTpcResiduals() override = default;

  int Init(PHCompositeNode *topNode) override;
  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int End(PHCompositeNode *topNode) override;

  /// Option for setting distortion correction calculation limits
  void setMaxTrackAlpha(float maxTAlpha) 
    { m_maxTAlpha = maxTAlpha;}
  void setMaxTrackBeta(float maxTBeta)
    { m_maxTBeta = maxTBeta; }
  void setMaxTrackResidualDrphi(float maxResidualDrphi) 
    { m_maxResidualDrphi = maxResidualDrphi;}
  
  void setMaxTrackResidualDz(float maxResidualDz)
    { m_maxResidualDz = maxResidualDz; }
  
  void setGridDimensions(const int phiBins, const int rBins, const int zBins);

  /// set to true to store evaluation histograms and ntuples
  void setSavehistograms( bool value ) { m_savehistograms = value; }
    
  /// output file name for evaluation histograms
  void setHistogramOutputfile(const std::string &outputfile) {m_histogramfilename = outputfile;}

  /// output file name for storing the space charge reconstruction matrices
  void setOutputfile(const std::string &outputfile) {m_outputfile = outputfile;}

  /// require micromegas to be present when extrapolating tracks to the TPC
  void setUseMicromegas( bool value )
  { m_useMicromegas = value; }
  
 private:

  int getNodes(PHCompositeNode *topNode);
  int createNodes(PHCompositeNode *topNode);

  int processTracks(PHCompositeNode *topNode);

  bool checkTrack(SvtxTrack* track);
  void processTrack(SvtxTrack* track);

  /// Calculates TPC residuals given an Acts::Propagation result to
  /// a TPC surface
  void calculateTpcResiduals(const Acts::BoundTrackParameters& params,
			     TrkrCluster* cluster);

  /// Propagates the silicon+MM track fit to the surface on which
  /// an available source link in the TPC exists, added from the stub
  /// matching propagation
  BoundTrackParamPtrResult propagateTrackState(
  const Acts::BoundTrackParameters& params, 
		     const SourceLink& sl);

  /// Gets distortion cell for identifying bins in TPC
  int getCell(const Acts::Vector3D& loc);
  
  void makeHistograms();
  SourceLink makeSourceLink(TrkrCluster* cluster);
  Acts::BoundTrackParameters makeTrackParams(SvtxTrack* track);
  Surface getSurface(TrkrDefs::cluskey cluskey,
		     TrkrDefs::subsurfkey);
      
  Surface getSiliconSurface(TrkrDefs::hitsetkey hitsetkey);
  Surface getTpcSurface(TrkrDefs::hitsetkey hitsetkey, TrkrDefs::subsurfkey surfkey);
  Surface getMMSurface(TrkrDefs::hitsetkey hitsetkey);

  /// Node information for Acts tracking geometry and silicon+MM
  /// track fit
  SvtxTrackMap *m_trackMap = nullptr;
  ActsTrackingGeometry *m_tGeometry = nullptr;
  TrkrClusterContainer *m_clusterContainer = nullptr;
  ActsSurfaceMaps *m_surfMaps = nullptr;

  float m_maxTAlpha = 0.6;
  float m_maxResidualDrphi = 0.5; // cm
  float m_maxTBeta = 1.5;
  float m_maxResidualDz = 0.5; // cm

  static constexpr float m_phiMin = 0;
  static constexpr float m_phiMax = 2. * M_PI;

  static constexpr float m_rMin = 20; // cm
  static constexpr float m_rMax = 78; // cm

  static constexpr int m_minClusCount = 10;

  /// Tpc geometry
  static constexpr unsigned int m_nLayersTpc = 48;
  static constexpr float m_zMin = -105.5; // cm
  static constexpr float m_zMax = 105.5;  // cm

  /// matrix container
  std::unique_ptr<TpcSpaceChargeMatrixContainer> m_matrix_container;
  
  // TODO: check if needed
  int m_event = 0;
  
  /// Counter for number of bad propagations from propagateTrackState()
  int m_nBadProps = 0;

  /// require micromegas to be present when extrapolating tracks to the TPC
  bool m_useMicromegas = true;

  std::string m_outputfile = "TpcSpaceChargeMatrices.root";

  /// Output root histograms
  bool m_savehistograms = false;
  TH2 *h_rphiResid = nullptr;
  TH2 *h_zResid = nullptr;
  TH2 *h_etaResidLayer = nullptr;
  TH2 *h_zResidLayer = nullptr;
  TH2 *h_etaResid = nullptr;
  TH1 *h_index = nullptr;
  TH2 *h_alpha = nullptr;
  TH2 *h_beta = nullptr;
  TTree *residTup = nullptr;

  /// delta rphi vs layer number
  TH2 *h_deltarphi_layer = nullptr;

  /// delta z vs layer number
  TH2 *h_deltaz_layer = nullptr;

  std::string m_histogramfilename = "PHTpcResiduals.root";
  std::unique_ptr<TFile> m_histogramfile = nullptr;

  /// For diagnostics
  double tanAlpha = 0;
  double tanBeta = 0;
  double drphi = 0;
  double dz = 0;
  double clusR = 0;
  double clusPhi = 0;
  double clusZ = 0;
  double statePhi = 0;
  double stateZ = 0;
  double stateRPhiErr = 0;
  double stateZErr = 0;
  double clusRPhiErr = 0;
  double clusZErr = 0;
  double stateR = 0;
  TrkrDefs::cluskey cluskey = 0;

};

#endif

