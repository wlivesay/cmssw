#include "L1Trigger/TrackFindingTracklet/interface/TrackMultiplexer.h"

#include <vector>
#include <deque>
#include <set>
#include <numeric>
#include <algorithm>

namespace trklet {

  TrackMultiplexer::TrackMultiplexer(const Setup* setup, const DataFormats* dataFormats, int region, const TTDTC& ttDTC)
      : setup_(setup), dataFormats_(dataFormats), region_(region), input_(setup_->tbNumChannelsTrack()) {
    // unified tracklet digitisation granularity
    baseUinv2R_ = setup->tbBaseInv2R();
    baseUphiT_ = setup->tbBasePhi0();
    baseUcot_ = setup->tbBaseCot();
    baseUzT_ = setup->tbBaseZ0();
    baseUr_ = setup->tbBaseR();
    baseUphi_ = setup->tbBasePhi();
    baseUz_ = setup->tbBaseZ();
    // DR input format digitisation granularity (identical to TMTT)
    baseLinv2R_ = dataFormats->base(Variable::inv2R, Process::tm);
    baseLphiT_ = dataFormats->base(Variable::phiT, Process::tm);
    baseLzT_ = dataFormats->base(Variable::zT, Process::tm);
    baseLr_ = dataFormats->base(Variable::r, Process::tm);
    baseLphi_ = dataFormats->base(Variable::phi, Process::tm);
    baseLz_ = dataFormats->base(Variable::z, Process::tm);
    baseLcot_ = baseLz_ / baseLr_;
    // Finer granularity (by powers of 2) than the TMTT one. Used to transform from Tracklet to TMTT base.
    baseHinv2R_ = baseLinv2R_ * std::pow(2, std::floor(std::log2(baseUinv2R_ / baseLinv2R_)));
    baseHphiT_ = baseLphiT_ * std::pow(2, std::floor(std::log2(baseUphiT_ / baseLphiT_)));
    baseHzT_ = baseLzT_ * std::pow(2, std::floor(std::log2(baseUzT_ / baseLzT_)));
    baseHr_ = baseLr_ * std::pow(2, std::floor(std::log2(baseUr_ / baseLr_)));
    baseHphi_ = baseLphi_ * std::pow(2, std::floor(std::log2(baseUphi_ / baseLphi_)));
    baseHz_ = baseLz_ * std::pow(2, std::floor(std::log2(baseUz_ / baseLz_)));
    baseHcot_ = baseLcot_ * std::pow(2, std::floor(std::log2(baseUcot_ / baseLcot_)));
    if (setup_->tmUseDTCStubs()) {
      // prep dtc stub container
      int size(0);
      for (int channel : ttDTC.tfpChannels())
        for (const tt::FrameStub& frame : ttDTC.stream(region, channel))
          if (frame.first.isNonnull())
            size++;
      dtc_.reserve(size);
      // fill dtc stub container
      for (int channel : ttDTC.tfpChannels())
        for (const tt::FrameStub& frame : ttDTC.stream(region, channel))
          if (frame.first.isNonnull())
            dtc_.push_back(frame);
    }
  }

  // read in and organize input tracks and stubs
  void TrackMultiplexer::consume(const tt::StreamsTrack& streamsTrack, const tt::StreamsStub& streamsStub) {
    const int offsetTrack = region_ * setup_->tbNumChannelsTrack();
    const int offsetStub = region_ * setup_->tbNumChannelsStub();
    // count tracks and stubs to reserve container
    int nTracks(0);
    int nStubs(0);
    for (int seedType = 0; seedType < setup_->tbNumChannelsTrack(); seedType++) {
      const int channelTrack = offsetTrack + seedType;
      const int offsetChannel = offsetStub + setup_->tbOffsetStub(seedType);
      const tt::StreamTrack& streamTrack = streamsTrack[channelTrack];
      input_[seedType].reserve(streamTrack.size());
      for (int frame = 0; frame < static_cast<int>(streamTrack.size()); frame++) {
        if (streamTrack[frame].first.isNull())
          continue;
        nTracks++;
        for (int layer = 0; layer < setup_->tbNumProjectionLayers(seedType); layer++)
          if (streamsStub[offsetChannel + layer][frame].first.isNonnull())
            nStubs++;
      }
    }
    stubs_.reserve(nStubs + nTracks * setup_->tbNumSeedingLayers());
    tracks_.reserve(nTracks);
    // store tracks and stubs
    for (int seedType = 0; seedType < setup_->tbNumChannelsTrack(); seedType++) {
      const int numP = setup_->tbNumProjectionLayers(seedType);
      const int channelTrack = offsetTrack + seedType;
      const int offsetChannel = offsetStub + setup_->tbOffsetStub(seedType);
      const tt::StreamTrack& streamTrack = streamsTrack[channelTrack];
      std::vector<Track*>& input = input_[seedType];
      for (int frame = 0; frame < static_cast<int>(streamTrack.size()); frame++) {
        const TTTrackRef& ttTrackRef = streamTrack[frame].first;
        if (ttTrackRef.isNull()) {
          input.push_back(nullptr);
          continue;
        }
        //convert track parameter
        const double offset = region_ * setup_->regRangePhiT();
        double inv2R = tt::digi(-ttTrackRef->rInv() / 2., baseUinv2R_);
        const double phi0U =
            tt::digi(tt::deltaPhi(ttTrackRef->phi() - offset + setup_->stubRangePhi() / 2.), baseUphiT_);
        const double phi0S = tt::digi(phi0U - setup_->stubRangePhi() / 2., baseUphiT_);
        double cot = tt::digi(ttTrackRef->tanL(), baseUcot_);
        double z0 = tt::digi(ttTrackRef->z0(), baseUzT_);
        double phiT = tt::digi(phi0S + inv2R * tt::digi(setup_->regChosenRofPhi(), baseUr_), baseUphiT_);
        double zT = tt::digi(z0 + cot * tt::digi(setup_->regChosenRofZ(), baseUr_), baseUzT_);
        // convert stubs
        std::vector<Stub*> stubs;
        stubs.reserve(setup_->tbNumSeedingLayers() + numP);
        for (int layer = 0; layer < numP; layer++) {
          const tt::FrameStub& frameStub = streamsStub[offsetChannel + layer][frame];
          const TTStubRef& ttStubRef = frameStub.first;
          if (ttStubRef.isNull())
            continue;
          const trackerDTC::SensorModule* sm = setup_->sensorModule(ttStubRef);
          const GlobalPoint gp = setup_->stubPosTB(frameStub, cot);
          const int widthR = setup_->tbWidthR(sm->type());
          const int widthRZ = sm->barrel() ? setup_->tbWidthZ() : setup_->tbWidthR();
          TTBV ttBV(frameStub.second);
          ttBV >>= widthRZ + setup_->tbWidthPhi() + widthR;
          const int stubId = ttBV.val(setup_->tmWidthStubId());
          bool psTilt = sm->barrel() ? sm->tilted() : sm->psModule();
          stubs_.emplace_back(ttStubRef, sm->layerIndexCombined(), stubId, gp.perp(), gp.phi(), gp.z(), psTilt);
          stubs.push_back(&stubs_.back());
        }
        // create fake seed stubs, since TrackBuilder doesn't output these stubs, required by the KF.
        for (int seedingLayer = 0; seedingLayer < setup_->tbNumSeedingLayers(); seedingLayer++) {
          const int channelStub = numP + seedingLayer;
          const tt::FrameStub& frameStub = streamsStub[offsetChannel + channelStub][frame];
          const TTStubRef& ttStubRef = frameStub.first;
          const trackerDTC::SensorModule* sm = setup_->sensorModule(ttStubRef);
          const GlobalPoint gp = setup_->stubPosTB(ttStubRef, cot, z0);
          const int stubId = TTBV(frameStub.second).val(setup_->tmWidthStubId());
          bool psTilt = sm->barrel() ? sm->tilted() : sm->psModule();
          stubs_.emplace_back(ttStubRef, sm->layerIndexCombined(), stubId, gp.perp(), 0, 0, psTilt);
          stubs.push_back(&stubs_.back());
        }
        // create track
        tracks_.emplace_back(ttTrackRef, seedType, inv2R, phiT, cot, zT, stubs);
        input.push_back(&tracks_.back());
      }
    }
    if (setup_->tmUseDTCStubs() || setup_->tmUseTTStubs()) {
      for (Track& track : tracks_) {
        // global stub coords
        for (Stub* stub : track.stubs_) {
          GlobalPoint gp;
          if (setup_->tmUseDTCStubs()) {
            auto via = [stub](const tt::FrameStub& fs) { return fs.first == stub->ttStubRef_; };
            const tt::FrameStub& fs = *std::find_if(dtc_.begin(), dtc_.end(), via);
            gp = setup_->stubPosDTC(fs, region_);
          } else
            gp = setup_->stubPosTT(stub->ttStubRef_);
          stub->r_ = gp.perp();
          stub->phi_ = tt::deltaPhi(gp.phi() - region_ * setup_->regRangePhiT());
          stub->z_ = gp.z();
        }
        // calc track parameter
        Stub* s0 = *std::next(track.stubs_.end(), -2);
        Stub* s1 = *std::next(track.stubs_.end(), -1);
        const double dH = s1->r_ - s0->r_;
        const double H1m0 = s1->r_ * s0->phi_;
        const double H0m1 = s0->r_ * s1->phi_;
        const double H3m2 = s1->r_ * s0->z_;
        const double H2m3 = s0->r_ * s1->z_;
        track.inv2R_ = (s1->phi_ - s0->phi_) / dH;
        track.cot_ = (s1->z_ - s0->z_) / dH;
        const double phi0 = (H1m0 - H0m1) / dH;
        const double z0 = (H3m2 - H2m3) / dH;
        track.phiT_ = phi0 + std::asin(setup_->regChosenRofPhi() * track.inv2R_);
        track.zT_ = z0 + std::asin(setup_->regChosenRofZ() * track.inv2R_) / track.inv2R_ * track.cot_;
        // calc residuals
        for (Stub* stub : track.stubs_) {
          stub->phi_ -= phi0 + std::asin(stub->r_ * track.inv2R_);
          stub->z_ -= z0 + std::asin(stub->r_ * track.inv2R_) / track.inv2R_ * track.cot_;
        }
      }
    }
  }

  // fill output products
  void TrackMultiplexer::produce(tt::StreamsTrack& streamsTrack, tt::StreamsStub& streamsStub) {
    // base transform into high precision KF format
    for (Track& track : tracks_) {
      track.inv2R_ = tt::redigi(track.inv2R_, baseUinv2R_, baseHinv2R_, setup_->widthDSPbu());
      track.phiT_ = tt::redigi(track.phiT_, baseUphiT_, baseHphiT_, setup_->widthDSPbu());
      track.cot_ = tt::redigi(track.cot_, baseUcot_, baseHcot_, setup_->widthDSPbu());
      track.zT_ = tt::redigi(track.zT_, baseUzT_, baseHzT_, setup_->widthDSPbu());
      for (Stub* stub : track.stubs_) {
        stub->r_ = tt::redigi(stub->r_, baseUr_, baseHr_, setup_->widthDSPbu());
        stub->phi_ = tt::redigi(stub->phi_, baseUphi_, baseHphi_, setup_->widthDSPbu());
        stub->z_ = tt::redigi(stub->z_, baseUz_, baseHz_, setup_->widthDSPbu());
      }
    }
    // base transform into TM format
    for (Track& track : tracks_) {
      // store track parameter shifts
      const double dinv2R = tt::digi(track.inv2R_ - tt::digi(track.inv2R_, baseLinv2R_), baseHinv2R_);
      const double dphiT = tt::digi(track.phiT_ - tt::digi(track.phiT_, baseLphiT_), baseHphiT_);
      const double dcot = track.cot_ - tt::digi(tt::digi(track.zT_, baseLzT_) / setup_->regChosenRofZ(), baseHcot_);
      const double dzT = tt::digi(track.zT_ - tt::digi(track.zT_, baseLzT_), baseHzT_);
      // shift track parameter;
      track.inv2R_ -= dinv2R;
      track.phiT_ -= dphiT;
      track.cot_ -= dcot;
      track.zT_ -= dzT;
      // adjust stub residuals by track parameter shifts
      for (Stub* stub : track.stubs_) {
        const double rz = tt::digi(stub->r_ - setup_->regChosenRofZ(), baseHr_);
        const double dz = tt::digi(dzT + rz * dcot, baseHz_);
        stub->r_ = tt::digi(stub->r_ - setup_->regChosenRofPhi(), baseHr_);
        const double dphi = tt::digi(dphiT + stub->r_ * dinv2R, baseHphi_);
        stub->phi_ = tt::digi(stub->phi_ + dphi, baseLphi_);
        stub->z_ = tt::digi(stub->z_ + dz, baseLz_);
      }
    }
    // range checks
    for (Track& track : tracks_) {
      if (!dataFormats_->format(Variable::inv2R, Process::tm).isCovered(track.inv2R_))
        track.valid_ = false;
      if (!dataFormats_->format(Variable::phiT, Process::tm).isCovered(track.phiT_))
        track.valid_ = false;
      if (!dataFormats_->format(Variable::zT, Process::tm).isCovered(track.zT_))
        track.valid_ = false;
      for (Stub* stub : track.stubs_) {
        if (!dataFormats_->format(Variable::r, Process::tm).isCovered(stub->r_))
          stub->valid_ = false;
        if (!dataFormats_->format(Variable::phi, Process::tm).isCovered(stub->phi_))
          stub->valid_ = false;
        if (!dataFormats_->format(Variable::z, Process::tm).isCovered(stub->z_))
          stub->valid_ = false;
      }
    }
    // emualte clock domain crossing
    static constexpr int ticksPerGap = 3;
    static constexpr int gapPos = 1;
    std::vector<std::deque<Track*>> streams(setup_->tbNumChannelsTrack());
    for (int seedType = 0; seedType < setup_->tbNumChannelsTrack(); seedType++) {
      int iTrack(0);
      std::deque<Track*>& stream = streams[seedType];
      const std::vector<Track*>& intput = input_[seedType];
      for (int tick = 0; iTrack < (int)intput.size(); tick++) {
        Track* track = tick % ticksPerGap != gapPos ? intput[iTrack++] : nullptr;
        stream.push_back(track && track->valid_ ? track : nullptr);
      }
    }
    // remove all gaps between end and last track
    for (std::deque<Track*>& stream : streams)
      for (auto it = stream.end(); it != stream.begin();)
        it = (*--it) ? stream.begin() : stream.erase(it);
    // route into single channel
    std::deque<Track*> accepted;
    std::vector<std::deque<Track*>> stacks(setup_->tbNumChannelsTrack());
    // clock accurate firmware emulation, each while trip describes one clock tick, one stub in and one stub out per tick
    auto empty = [](const std::deque<Track*>& tracks) { return tracks.empty(); };
    while (!std::all_of(streams.begin(), streams.end(), empty) || !std::all_of(stacks.begin(), stacks.end(), empty)) {
      // fill input fifos
      for (int seedType = 0; seedType < setup_->tbNumChannelsTrack(); seedType++) {
        Track* track = pop_front(streams[seedType]);
        if (track)
          stacks[seedType].push_back(track);
      }
      // merge input fifos to one stream, prioritizing lower input channel over higher channel, affects DR
      bool nothingToRoute(true);
      for (int seedType : setup_->tmMuxOrder()) {
        Track* track = pop_front(stacks[seedType]);
        if (track) {
          nothingToRoute = false;
          accepted.push_back(track);
          break;
        }
      }
      if (nothingToRoute)
        accepted.push_back(nullptr);
    }
    // truncate if desired
    if (setup_->enableTruncation() && static_cast<int>(accepted.size()) > setup_->numFrames())
      accepted.resize(setup_->numFrames());
    // remove all gaps between end and last track
    for (auto it = accepted.end(); it != accepted.begin();)
      it = (*--it) ? accepted.begin() : accepted.erase(it);
    // store helper
    auto frameTrack = [this](Track* track) { return track->valid_ ? track->frame(dataFormats_) : tt::FrameTrack(); };
    auto frameStub = [this](Track* track, int layer) {
      const auto it = std::find_if(
          track->stubs_.begin(), track->stubs_.end(), [layer](Stub* stub) { return stub->layer_ == layer; });
      if (!track->valid_ || it == track->stubs_.end() || !(*it)->valid_)
        return tt::FrameStub();

      return (*it)->frame(dataFormats_);
    };
    const int offsetStub = region_ * setup_->tmNumLayers();
    // fill output tracks and stubs
    streamsTrack[region_].reserve(accepted.size());
    for (int layer = 0; layer < setup_->tmNumLayers(); layer++)
      streamsStub[offsetStub + layer].reserve(accepted.size());
    for (Track* track : accepted) {
      if (!track) {  // fill gaps
        streamsTrack[region_].emplace_back(tt::FrameTrack());
        for (int layer = 0; layer < setup_->tmNumLayers(); layer++)
          streamsStub[offsetStub + layer].emplace_back(tt::FrameStub());
        continue;
      }
      streamsTrack[region_].emplace_back(frameTrack(track));
      for (int layer = 0; layer < setup_->tmNumLayers(); layer++)
        streamsStub[offsetStub + layer].emplace_back(frameStub(track, layer));
    }
  }

  // remove and return first element of deque, returns nullptr if empty
  template <class T>
  T* TrackMultiplexer::pop_front(std::deque<T*>& ts) const {
    T* t = nullptr;
    if (!ts.empty()) {
      t = ts.front();
      ts.pop_front();
    }
    return t;
  }

}  // namespace trklet
