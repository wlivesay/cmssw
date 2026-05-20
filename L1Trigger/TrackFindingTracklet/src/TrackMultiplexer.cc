#include "L1Trigger/TrackFindingTracklet/interface/TrackMultiplexer.h"

#include <vector>
#include <deque>
#include <set>
#include <numeric>
#include <algorithm>

namespace trklet {

  TrackMultiplexer::TrackMultiplexer(const Setup* setup, const DataFormats* dataFormats, int region, const TTDTC& ttDTC)
      : setup_(setup), dataFormats_(dataFormats), region_(region), input_(setup_->tbNumSeedTypes()) {
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
    const int offsetTrack = region_ * setup_->tbNumSeedTypes();
    // count tracks and stubs to reserve container
    int nTracks(0);
    int nStubs(0);
    for (int seedType = 0; seedType < setup_->tbNumSeedTypes(); seedType++) {
      const int channelTrack = offsetTrack + seedType;
      const int offsetStub = channelTrack * setup_->tbNumLayers();
      const tt::StreamTrack& streamTrack = streamsTrack[channelTrack];
      input_[seedType].reserve(streamTrack.size());
      for (int frame = 0; frame < static_cast<int>(streamTrack.size()); frame++) {
        if (streamTrack[frame].first.isNull())
          continue;
        nTracks++;
        for (int layer = 0; layer < setup_->tbNumLayers(); layer++)
          if (streamsStub[offsetStub + layer][frame].first.isNonnull())
            nStubs++;
      }
    }
    stubs_.reserve(nStubs);
    tracks_.reserve(nTracks);
    // store tracks and stubs
    for (int seedType = 0; seedType < setup_->tbNumSeedTypes(); seedType++) {
      const std::vector<int>& layersSeed = setup_->tbSeedLayers(seedType);
      const std::vector<int>& layersProj = setup_->tbProjectionLayers(seedType);
      const int channelTrack = offsetTrack + seedType;
      const int offsetStub = channelTrack * setup_->tbNumLayers();
      std::vector<Track*>& input = input_[seedType];
      const tt::StreamTrack streamTrack = streamsTrack[channelTrack];
      for (int frame = 0; frame < static_cast<int>(streamTrack.size()); frame++) {
        const tt::FrameTrack frameTrack = streamTrack[frame];
        const TTTrackRef& ttTrackRef = frameTrack.first;
        if (ttTrackRef.isNull()) {
          input.push_back(nullptr);
          continue;
        }
        // parse track bits
        TTBV ttBV(frameTrack.second);
        const double cot = ttBV.val(setup_->tbWidthCot(), 0, true) * setup_->tbBaseCot();
        ttBV >>= setup_->tbWidthCot();
        const double z0 = ttBV.val(setup_->tbWidthZ0(), 0, true) * setup_->tbBaseZ0();
        ttBV >>= setup_->tbWidthZ0();
        const double phi0 = ttBV.val(setup_->tbWidthPhi0()) * setup_->tbBasePhi0() - setup_->stubRangePhi() / 2.;
        ttBV >>= setup_->tbWidthPhi0();
        const double inv2R = ttBV.val(setup_->tbWidthInv2R(), 0, true) * setup_->tbBaseInv2R();
        // parse seeds
        std::vector<Stub*> stubs(setup_->tmNumLayers(), nullptr);
        for (int iSeed = 0; iSeed < setup_->tbNumSeedingLayers(); iSeed++) {
          const int layer = toLayer(layersSeed[iSeed]);
          const tt::FrameStub& frameStub = streamsStub[offsetStub + layer][frame];
          const TTStubRef& ttStubRef = frameStub.first;
          const trackerDTC::SensorModule* sm = setup_->sensorModule(ttStubRef);
          TTBV ttBV(frameStub.second);
          const int stubId = TTBV(frameStub.second).val(setup_->tbWidthStubId());
          stubs_.emplace_back(ttStubRef, sm, stubId);
          stubs[layer] = &stubs_.back();
        }
        // parse projections
        for (int iProj = 0; iProj < setup_->tbNumProjectionLayers(seedType); iProj++) {
          const int layer = toLayer(layersProj[iProj]);
          const tt::FrameStub& frameStub = streamsStub[offsetStub + layer][frame];
          const TTStubRef& ttStubRef = frameStub.first;
          if (ttStubRef.isNull())
            continue;
          const trackerDTC::SensorModule* sm = setup_->sensorModule(ttStubRef);
          const trackerDTC::SensorModule::Type type = sm->type();
          const int widthR = setup_->tbWidthR(type);
          const int widthPhi = setup_->tbWidthPhi();
          const int widthRZ = sm->barrel() ? setup_->tbWidthZ() : setup_->tbWidthR();
          const double baseR = setup_->stubBaseR(type);
          const double basePhi = setup_->tbBasePhi(sm->layerIndexCombined());
          const double baseRZ = setup_->tbBaseZ(sm->layerIndex());
          TTBV ttBV(frameStub.second);
          const double z = ttBV.val(widthRZ, 0, true) * baseRZ;
          ttBV >>= widthRZ;
          const double phi = ttBV.val(widthPhi, 0, true) * basePhi;
          ttBV >>= widthPhi;
          const double r = ttBV.val(widthR, 0, sm->barrel()) * baseR;
          ttBV >>= widthR;
          const int stubId = ttBV.val(setup_->tbWidthStubId());
          stubs_.emplace_back(ttStubRef, sm, stubId, r, phi, z);
          stubs[layer] = &stubs_.back();
        }
        // create track
        tracks_.emplace_back(ttTrackRef, seedType, inv2R, phi0, cot, z0, stubs);
        input.push_back(&tracks_.back());
      }
    }
  }

  // fill output products
  void TrackMultiplexer::produce(tt::StreamsTrack& streamsTrack, tt::StreamsStub& streamsStub) {
    // unify stubs and tracks
    for (Track& track : tracks_) {
      const double chosenRofPhi = tt::digi(setup_->regChosenRofPhi(), baseUr_);
      const double chosenRofZ = tt::digi(setup_->regChosenRofZ(), baseUr_);
      track.inv2R_ = -track.inv2R_;
      track.phiT_ = tt::digiR(track.phiT_ + track.inv2R_ * chosenRofPhi, baseUphiT_);
      track.zT_ = tt::digiR(track.zT_ + track.cot_ * chosenRofZ, baseUzT_);
      for (int layerId : setup_->tbSeedLayers(track.seedType_)) {
        Stub* stub = track.stubs_[toLayer(layerId)];
        const trackerDTC::SensorModule* sm = stub->sm_;
        const int layerIndex = sm->layerIndex();
        if (sm->barrel()) {
          const double z = track.zT_ + (stub->r_ - chosenRofZ) * track.cot_;
          if (std::abs(z) > setup_->tbMinZ() && layerIndex == 0)
            stub->r_ = tt::digiR(setup_->tbInnerRadius() - setup_->regChosenRofPhi(), baseUr_);
          else
            stub->r_ = tt::digiR(setup_->stubLayerR(layerIndex) - setup_->regChosenRofPhi(), baseUr_);
        } else {
          const double z = tt::digi((sm->side() ? 1. : -1.) * setup_->stubDiskZ(layerIndex), baseUzT_);
          const double invCot = tt::digi(1. / tt::digi(std::abs(track.cot_), baseUcot_), setup_->tmBaseInvCot());
          stub->r_ = tt::digiR((z - track.zT_) * invCot, baseUr_);
        }
      }
      for (int layerId : setup_->tbProjectionLayers(track.seedType_)) {
        Stub* stub = track.stubs_[toLayer(layerId)];
        if (!stub)
          continue;
        const trackerDTC::SensorModule* sm = stub->sm_;
        const int layerIndex = sm->layerIndex();
        if (!sm->barrel())
          stub->z_ = tt::digiR(-stub->z_ * track.cot_, baseUz_);
        else
          stub->r_ = tt::digiR(
              stub->r_ + tt::digiR(setup_->stubLayerR(layerIndex) - setup_->regChosenRofPhi(), baseUr_), baseUr_);
        if (sm->type() == trackerDTC::SensorModule::Disk2S)
          stub->r_ = tt::digiR(setup_->stubDiskR(layerIndex, stub->r_) - setup_->regChosenRofPhi(), baseUr_);
      }
    }
    // recalculate tracks and stubs from DTC or TT Stubs
    if (setup_->tmUseDTCStubs() || setup_->tmUseTTStubs()) {
      for (Track& track : tracks_) {
        // global stub coords
        for (Stub* stub : track.stubs_) {
          if (!stub)
            continue;
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
        };
        // calc track parameter
        const std::vector<int>& layersSeed = setup_->tbSeedLayers(track.seedType_);
        Stub* s0 = track.stubs_[toLayer(layersSeed[0])];
        Stub* s1 = track.stubs_[toLayer(layersSeed[1])];
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
          if (!stub)
            continue;
          stub->phi_ -= phi0 + std::asin(stub->r_ * track.inv2R_);
          stub->z_ -= z0 + std::asin(stub->r_ * track.inv2R_) / track.inv2R_ * track.cot_;
          stub->r_ = stub->r_ - setup_->regChosenRofPhi();
        }
      }
    }
    // base transform into high precision format
    for (Track& track : tracks_) {
      track.inv2R_ = tt::redigi(track.inv2R_, baseUinv2R_, baseHinv2R_, setup_->widthDSPbu());
      track.phiT_ = tt::redigi(track.phiT_, baseUphiT_, baseHphiT_, setup_->widthDSPbu());
      track.cot_ = tt::redigi(track.cot_, baseUcot_, baseHcot_, setup_->widthDSPbu());
      track.zT_ = tt::redigi(track.zT_, baseUzT_, baseHzT_, setup_->widthDSPbu());
      for (Stub* stub : track.stubs_) {
        if (!stub)
          continue;
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
        if (!stub)
          continue;
        const double chosenR = tt::digi(setup_->regChosenRofZ() - setup_->regChosenRofPhi(), baseHr_);
        const double rz = tt::digi(stub->r_ - chosenR, baseHr_);
        const double dz = tt::digi(dzT + rz * dcot, baseHz_);
        const double dphi = tt::digi(dphiT + stub->r_ * dinv2R, baseHphi_);
        stub->phi_ = tt::digi(stub->phi_ + dphi, baseLphi_);
        stub->z_ = tt::digi(stub->z_ + dz, baseLz_);
      }
    }
    // range checks
    for (std::vector<Track*>& stream : input_) {
      for (Track*& track : stream) {
        if (!track)
          continue;
        bool valid(true);
        if (!dataFormats_->format(Variable::inv2R, Process::tm).isCovered(track->inv2R_))
          valid = false;
        if (!dataFormats_->format(Variable::phiT, Process::tm).isCovered(track->phiT_))
          valid = false;
        if (!dataFormats_->format(Variable::zT, Process::tm).isCovered(track->zT_))
          valid = false;
        if (!valid) {
          track = nullptr;
          continue;
        }
        for (Stub*& stub : track->stubs_) {
          if (!stub)
            continue;
          bool valid(true);
          if (!dataFormats_->format(Variable::r, Process::tm).isCovered(stub->r_))
            valid = false;
          if (!dataFormats_->format(Variable::phi, Process::tm).isCovered(stub->phi_))
            valid = false;
          if (!dataFormats_->format(Variable::z, Process::tm).isCovered(stub->z_))
            valid = false;
          if (!valid)
            stub = nullptr;
        }
      }
    }
    // emualte clock domain crossing
    static constexpr int ticksPerGap = 3;
    static constexpr int gapPos = 1;
    std::vector<std::deque<Track*>> streams(setup_->tbNumSeedTypes());
    for (int seedType = 0; seedType < setup_->tbNumSeedTypes(); seedType++) {
      int iTrack(0);
      std::deque<Track*>& stream = streams[seedType];
      const std::vector<Track*>& intput = input_[seedType];
      for (int tick = 0; iTrack < (int)intput.size(); tick++)
        stream.push_back(tick % ticksPerGap != gapPos ? intput[iTrack++] : nullptr);
    }
    // remove all gaps between end and last track
    for (std::deque<Track*>& stream : streams)
      for (auto it = stream.end(); it != stream.begin();)
        it = (*--it) ? stream.begin() : stream.erase(it);
    // route into single channel
    std::deque<Track*> accepted;
    std::vector<std::deque<Track*>> stacks(setup_->tbNumSeedTypes());
    // clock accurate firmware emulation, each while trip describes one clock tick, one stub in and one stub out per tick
    auto empty = [](const std::deque<Track*>& tracks) { return tracks.empty(); };
    while (!std::all_of(streams.begin(), streams.end(), empty) || !std::all_of(stacks.begin(), stacks.end(), empty)) {
      // fill input fifos
      for (int seedType = 0; seedType < setup_->tbNumSeedTypes(); seedType++) {
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
    auto toFrame = [this](Stub* stub) { return stub ? stub->frame(dataFormats_) : tt::FrameStub(); };
    // prep output container
    const int offset = region_ * setup_->tmNumLayers();
    tt::StreamTrack& streamTrack = streamsTrack[region_];
    streamTrack.reserve(accepted.size());
    for (int layer = 0; layer < setup_->tmNumLayers(); layer++)
      streamsStub[offset + layer].reserve(accepted.size());
    // fill output tracks and stubs
    for (Track* track : accepted) {
      if (!track) {  // fill gaps
        streamTrack.emplace_back(tt::FrameTrack());
        for (int layer = 0; layer < setup_->tmNumLayers(); layer++)
          streamsStub[offset + layer].emplace_back(tt::FrameStub());
        continue;
      }
      streamTrack.emplace_back(track->frame(dataFormats_));
      int layer(0);
      for (Stub* stub : track->stubs_)
        streamsStub[offset + layer++].emplace_back(toFrame(stub));
    }
  }

  // turns layerId [1-6, 11-15] into layerIndexCombined [0-10]
  int TrackMultiplexer::toLayer(int layerId) const {
    static constexpr int offsetBarrel = 1;
    static constexpr int offsetDisk = 11;
    static constexpr int numBarrel = 6;
    return layerId < offsetDisk ? layerId - offsetBarrel : layerId - offsetDisk + numBarrel;
  };

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
