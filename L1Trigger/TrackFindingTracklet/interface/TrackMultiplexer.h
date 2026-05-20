#ifndef L1Trigger_TrackFindingTracklet_TrackMultiplexer_h
#define L1Trigger_TrackFindingTracklet_TrackMultiplexer_h

#include "L1Trigger/TrackFindingTracklet/interface/Setup.h"
#include "L1Trigger/TrackFindingTracklet/interface/DataFormats.h"
#include "DataFormats/L1TrackTrigger/interface/TTDTC.h"

#include <vector>
#include <deque>

namespace trklet {

  /*! \class  trklet::TrackMultiplexer
   *  \brief  Class to emulate transformation of tracklet tracks and stubs into TMTT format
   *          and routing of seed type streams into single stream
   *  \author Thomas Schuh
   *  \date   2023, Jan
   */
  class TrackMultiplexer {
  public:
    TrackMultiplexer(const Setup*, const DataFormats*, int, const TTDTC&);
    ~TrackMultiplexer() = default;
    // read in and organize input tracks and stubs
    void consume(const tt::StreamsTrack&, const tt::StreamsStub&);
    // fill output products
    void produce(tt::StreamsTrack&, tt::StreamsStub&);

  private:
    // turns layerId [1-6, 11-15] into layerIndexCombined [0-10]
    int toLayer(int) const;
    struct Stub {
      Stub(const TTStubRef& ttStubRef, const trackerDTC::SensorModule* sm, int stubId, double r, double phi, double z)
          : ttStubRef_(ttStubRef), sm_(sm), stubId_(stubId), r_(r), phi_(phi), z_(z) {
        bool psTilt = sm->barrel() ? sm->tilted() : sm->psModule();
        stubId_ = 2 * stubId_ + (psTilt ? 1 : 0);
      }
      Stub(const TTStubRef& ttStubRef, const trackerDTC::SensorModule* sm, int stubId)
          : Stub(ttStubRef, sm, stubId, 0., 0., 0.) {}
      tt::FrameStub frame(const DataFormats* df) const { return StubTM(ttStubRef_, df, stubId_, r_, phi_, z_).frame(); }
      TTStubRef ttStubRef_;
      const trackerDTC::SensorModule* sm_;
      // tracklet stub id, used to identify duplicates
      int stubId_;
      // radius w.r.t. chosenRofPhi in cm
      double r_;
      // phi residual in rad
      double phi_;
      // z residual in cm
      double z_;
    };
    struct Track {
      Track(const TTTrackRef& ttTrackRef,
            int seedType,
            double inv2R,
            double phi0,
            double cot,
            double z0,
            const std::vector<Stub*>& stubs)
          : ttTrackRef_(ttTrackRef),
            seedType_(seedType),
            inv2R_(inv2R),
            phiT_(phi0),
            cot_(cot),
            zT_(z0),
            stubs_(stubs) {}
      tt::FrameTrack frame(const DataFormats* df) const { return TrackTM(ttTrackRef_, df, inv2R_, phiT_, zT_).frame(); }
      TTTrackRef ttTrackRef_;
      int seedType_;
      double inv2R_;
      double phiT_;
      double cot_;
      double zT_;
      std::vector<Stub*> stubs_;
    };
    // remove and return first element of deque, returns nullptr if empty
    template <class T>
    T* pop_front(std::deque<T*>& ts) const;
    // true if truncation is enbaled
    bool enableTruncation_;
    // stub residuals are recalculated from seed parameter and TTStub position
    bool useTTStubResiduals_;
    // track parameter are recalculated from seed TTStub positions
    bool useTTStubParameters_;
    //
    bool applyNonLinearCorrection_;
    // provides run-time constants
    const Setup* setup_;
    // provides dataformats
    const DataFormats* dataFormats_;
    // processing region (0 - 8) aka processing phi nonant
    const int region_;
    // storage of input tracks
    std::vector<Track> tracks_;
    // storage of input stubs
    std::vector<Stub> stubs_;
    // h/w liked organized pointer to input tracks
    std::vector<std::vector<Track*>> input_;
    // DTC stubs
    std::vector<tt::FrameStub> dtc_;
    // unified tracklet digitisation granularity
    double baseUinv2R_;
    double baseUphiT_;
    double baseUcot_;
    double baseUzT_;
    double baseUr_;
    double baseUphi_;
    double baseUz_;
    // KF input format digitisation granularity (identical to TMTT)
    double baseLinv2R_;
    double baseLphiT_;
    double baseLzT_;
    double baseLr_;
    double baseLphi_;
    double baseLz_;
    double baseLcot_;
    // Finer granularity (by powers of 2) than the TMTT one. Used to transform from Tracklet to TMTT base.
    double baseHinv2R_;
    double baseHphiT_;
    double baseHzT_;
    double baseHr_;
    double baseHphi_;
    double baseHz_;
    double baseHcot_;
  };

}  // namespace trklet

#endif
