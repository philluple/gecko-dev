/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_GFX_COMPOSITORMANAGERCHILD_H
#define MOZILLA_GFX_COMPOSITORMANAGERCHILD_H

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t, uint64_t
#include "mozilla/Atomics.h"
#include "mozilla/Attributes.h"                    // for override
#include "mozilla/RefPtr.h"                        // for already_AddRefed
#include "mozilla/StaticPtr.h"                     // for StaticRefPtr
#include "mozilla/layers/CompositableForwarder.h"  // for FwdTransactionCounter
#include "mozilla/layers/PCompositorManagerChild.h"

namespace mozilla {
namespace layers {

class CompositorBridgeChild;
class CompositorManagerParent;
class WebRenderLayerManager;

class CompositorManagerChild : public PCompositorManagerChild {
  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(CompositorManagerChild, override)

 public:
  static bool IsInitialized(uint64_t aProcessToken);
  static void InitSameProcess(uint32_t aNamespace, uint64_t aProcessToken);
  static bool Init(Endpoint<PCompositorManagerChild>&& aEndpoint,
                   uint32_t aNamespace, uint64_t aProcessToken = 0);
  static void Shutdown();
  static void OnGPUProcessLost(uint64_t aProcessToken);

  static bool CreateContentCompositorBridge(uint32_t aNamespace);

  static already_AddRefed<CompositorBridgeChild> CreateWidgetCompositorBridge(
      uint64_t aProcessToken, WebRenderLayerManager* aLayerManager,
      uint32_t aNamespace, CSSToLayoutDeviceScale aScale,
      const CompositorOptions& aOptions, bool aUseExternalSurfaceSize,
      const gfx::IntSize& aSurfaceSize, uint64_t aInnerWindowId);

  static already_AddRefed<CompositorBridgeChild>
  CreateSameProcessWidgetCompositorBridge(WebRenderLayerManager* aLayerManager,
                                          uint32_t aNamespace);

  static CompositorManagerChild* GetInstance() {
    MOZ_ASSERT(NS_IsMainThread());
    return sInstance;
  }

  // Threadsafe way to snapshot the current compositor process info from another
  // thread.
  static mozilla::ipc::EndpointProcInfo GetCompositorProcInfo();

  bool CanSend() const {
    MOZ_ASSERT(NS_IsMainThread());
    return mCanSend;
  }

  bool SameProcess() const {
    MOZ_ASSERT(NS_IsMainThread());
    return mSameProcess;
  }

  uint32_t GetNextResourceId() {
    MOZ_ASSERT(NS_IsMainThread());
    return ++mResourceId;
  }

  uint32_t GetNamespace() const { return mNamespace; }

  bool OwnsExternalImageId(const wr::ExternalImageId& aId) const {
    return mNamespace == static_cast<uint32_t>(wr::AsUint64(aId) >> 32);
  }

  wr::ExternalImageId GetNextExternalImageId() {
    uint64_t id = GetNextResourceId();
    MOZ_RELEASE_ASSERT(id != 0);
    id |= (static_cast<uint64_t>(mNamespace) << 32);
    return wr::ToExternalImageId(id);
  }

  void ActorDestroy(ActorDestroyReason aReason) override;

  void HandleFatalError(const char* aMsg) override;

  void ProcessingError(Result aCode, const char* aReason) override;

  bool ShouldContinueFromReplyTimeout() override;

  mozilla::ipc::IPCResult RecvNotifyWebRenderError(
      const WebRenderError&& aError);

  FwdTransactionCounter& GetFwdTransactionCounter() {
    return mFwdTransactionCounter;
  }

  void SetSyncIPCStartTimeStamp();
  void ClearSyncIPCStartTimeStamp();

 private:
  static StaticRefPtr<CompositorManagerChild> sInstance;

  CompositorManagerChild(uint64_t aProcessToken, uint32_t aNamespace,
                         bool aSameProcess);

  virtual ~CompositorManagerChild() = default;

  void SetReplyTimeout();

  uint64_t mProcessToken;
  uint32_t mNamespace;
  uint32_t mResourceId;
  bool mCanSend;
  bool mSameProcess;
  FwdTransactionCounter mFwdTransactionCounter;
  // Used to extend sync IPC reply timeout
  Maybe<TimeStamp> mSyncIPCStartTimeStamp;
};

}  // namespace layers
}  // namespace mozilla

#endif
