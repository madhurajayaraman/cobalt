// Copyright 2026 The Cobalt Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "third_party/blink/renderer/modules/cobalt/h5vcc_updater/h_5_vcc_updater.h"

#include "base/test/gmock_callback_support.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_tester.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_core.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_testing.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/testing/page_test_base.h"

namespace blink {

using ::testing::_;

class FakeH5vccUpdaterService
    : public h5vcc_updater::mojom::blink::H5vccUpdater {
 public:
  FakeH5vccUpdaterService() : receiver_(this) {}
  ~FakeH5vccUpdaterService() override = default;

  void BindRequest(mojo::ScopedMessagePipeHandle handle) {
    DCHECK(!receiver_.is_bound());
    receiver_.Bind(
        mojo::PendingReceiver<h5vcc_updater::mojom::blink::H5vccUpdater>(
            std::move(handle)));
    receiver_.set_disconnect_handler(WTF::BindOnce(
        &FakeH5vccUpdaterService::OnConnectionError, WTF::Unretained(this)));
  }

  void OnConnectionError() { receiver_.reset(); }

  MOCK_METHOD(void, GetUpdaterChannel, (GetUpdaterChannelCallback), (override));
  MOCK_METHOD(void,
              SetUpdaterChannel,
              (const String&, SetUpdaterChannelCallback),
              (override));
  MOCK_METHOD(void, GetUpdateStatus, (GetUpdateStatusCallback), (override));
  MOCK_METHOD(void,
              ResetInstallations,
              (ResetInstallationsCallback),
              (override));
  MOCK_METHOD(void,
              GetInstallationIndex,
              (GetInstallationIndexCallback),
              (override));
  MOCK_METHOD(void,
              GetAllowSelfSignedPackages,
              (GetAllowSelfSignedPackagesCallback),
              (override));
  MOCK_METHOD(void,
              SetAllowSelfSignedPackages,
              (bool, SetAllowSelfSignedPackagesCallback),
              (override));
  MOCK_METHOD(void,
              GetUpdateServerUrl,
              (GetUpdateServerUrlCallback),
              (override));
  MOCK_METHOD(void,
              SetUpdateServerUrl,
              (const String&, SetUpdateServerUrlCallback),
              (override));
  MOCK_METHOD(void,
              GetRequireNetworkEncryption,
              (GetRequireNetworkEncryptionCallback),
              (override));
  MOCK_METHOD(void,
              SetRequireNetworkEncryption,
              (bool, SetRequireNetworkEncryptionCallback),
              (override));
  MOCK_METHOD(void,
              GetLibrarySha256,
              (uint16_t, GetLibrarySha256Callback),
              (override));

 private:
  mojo::Receiver<h5vcc_updater::mojom::blink::H5vccUpdater> receiver_;
};

class H5vccUpdaterTest : public PageTestBase {
 public:
  H5vccUpdaterTest()
      : h5vcc_updater_service_(
            std::make_unique<
                ::testing::StrictMock<FakeH5vccUpdaterService>>()) {}

  void SetUp() override {
    PageTestBase::SetUp(gfx::Size());

    GetFrame().DomWindow()->GetBrowserInterfaceBroker().SetBinderForTesting(
        h5vcc_updater::mojom::blink::H5vccUpdater::Name_,
        WTF::BindRepeating(&FakeH5vccUpdaterService::BindRequest,
                           WTF::Unretained(h5vcc_updater_service())));
  }

  void TearDown() override {
    GetFrame().DomWindow()->GetBrowserInterfaceBroker().SetBinderForTesting(
        h5vcc_updater::mojom::blink::H5vccUpdater::Name_, {});
  }

  ::testing::StrictMock<FakeH5vccUpdaterService>* h5vcc_updater_service() {
    return h5vcc_updater_service_.get();
  }

 private:
  std::unique_ptr<::testing::StrictMock<FakeH5vccUpdaterService>>
      h5vcc_updater_service_;
};

TEST_F(H5vccUpdaterTest, ConstructDestroy) {
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);
  ASSERT_TRUE(h5vcc_updater);
}

TEST_F(H5vccUpdaterTest, GetUpdaterChannel) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  const String kChannel = "test-channel";
  EXPECT_CALL(*h5vcc_updater_service(), GetUpdaterChannel(_))
      .WillOnce(base::test::RunOnceCallback<0>(kChannel));

  ScriptPromise promise = h5vcc_updater->getUpdaterChannel(
      scope.GetScriptState(), scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
  v8::Local<v8::Value> v8_value = tester.Value().V8Value();
  EXPECT_TRUE(v8_value->IsString());
  EXPECT_EQ(kChannel, ToCoreString(v8_value.As<v8::String>()));
}

TEST_F(H5vccUpdaterTest, GetUpdateStatus) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  const String kStatus = "up-to-date";
  EXPECT_CALL(*h5vcc_updater_service(), GetUpdateStatus(_))
      .WillOnce(base::test::RunOnceCallback<0>(kStatus));

  ScriptPromise promise = h5vcc_updater->getUpdateStatus(
      scope.GetScriptState(), scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
  v8::Local<v8::Value> v8_value = tester.Value().V8Value();
  EXPECT_TRUE(v8_value->IsString());
  EXPECT_EQ(kStatus, ToCoreString(v8_value.As<v8::String>()));
}

TEST_F(H5vccUpdaterTest, GetInstallationIndex) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  const uint16_t kIndex = 42;
  EXPECT_CALL(*h5vcc_updater_service(), GetInstallationIndex(_))
      .WillOnce(base::test::RunOnceCallback<0>(kIndex));

  ScriptPromise promise = h5vcc_updater->getInstallationIndex(
      scope.GetScriptState(), scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
  v8::Local<v8::Value> v8_value = tester.Value().V8Value();
  EXPECT_TRUE(v8_value->IsNumber());
  EXPECT_EQ(kIndex, v8_value.As<v8::Number>()->Value());
}

TEST_F(H5vccUpdaterTest, SetUpdaterChannel) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  const String kChannel = "new-channel";
  EXPECT_CALL(*h5vcc_updater_service(), SetUpdaterChannel(kChannel, _))
      .WillOnce(base::test::RunOnceCallback<1>());

  ScriptPromise promise = h5vcc_updater->setUpdaterChannel(
      scope.GetScriptState(), kChannel, scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
}

TEST_F(H5vccUpdaterTest, ResetInstallations) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  EXPECT_CALL(*h5vcc_updater_service(), ResetInstallations(_))
      .WillOnce(base::test::RunOnceCallback<0>());

  ScriptPromise promise = h5vcc_updater->resetInstallations(
      scope.GetScriptState(), scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
}

TEST_F(H5vccUpdaterTest, GetLibrarySha256) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  const uint16_t kIndex = 1;
  const String kSha256 = "sha256-hash";
  EXPECT_CALL(*h5vcc_updater_service(), GetLibrarySha256(kIndex, _))
      .WillOnce(base::test::RunOnceCallback<1>(kSha256));

  ScriptPromise promise = h5vcc_updater->getLibrarySha256(
      scope.GetScriptState(), kIndex, scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
  v8::Local<v8::Value> v8_value = tester.Value().V8Value();
  EXPECT_TRUE(v8_value->IsString());
  EXPECT_EQ(kSha256, ToCoreString(v8_value.As<v8::String>()));
}

TEST_F(H5vccUpdaterTest, GetAllowSelfSignedPackages) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  EXPECT_CALL(*h5vcc_updater_service(), GetAllowSelfSignedPackages(_))
      .WillOnce(base::test::RunOnceCallback<0>(true));

  ScriptPromise promise = h5vcc_updater->getAllowSelfSignedPackages(
      scope.GetScriptState(), scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
  v8::Local<v8::Value> v8_value = tester.Value().V8Value();
  EXPECT_TRUE(v8_value->IsBoolean());
  EXPECT_TRUE(v8_value.As<v8::Boolean>()->Value());
}

TEST_F(H5vccUpdaterTest, GetUpdateServerUrl) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  const String kUrl = "https://example.com/update";
  EXPECT_CALL(*h5vcc_updater_service(), GetUpdateServerUrl(_))
      .WillOnce(base::test::RunOnceCallback<0>(kUrl));

  ScriptPromise promise = h5vcc_updater->getUpdateServerUrl(
      scope.GetScriptState(), scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
  v8::Local<v8::Value> v8_value = tester.Value().V8Value();
  EXPECT_TRUE(v8_value->IsString());
  EXPECT_EQ(kUrl, ToCoreString(v8_value.As<v8::String>()));
}

TEST_F(H5vccUpdaterTest, GetRequireNetworkEncryption) {
  V8TestingScope scope;
  auto* window = GetFrame().DomWindow();
  auto* h5vcc_updater = MakeGarbageCollected<H5vccUpdater>(*window);

  EXPECT_CALL(*h5vcc_updater_service(), GetRequireNetworkEncryption(_))
      .WillOnce(base::test::RunOnceCallback<0>(true));

  ScriptPromise promise = h5vcc_updater->getRequireNetworkEncryption(
      scope.GetScriptState(), scope.GetExceptionState());
  ScriptPromiseTester tester(scope.GetScriptState(), promise);
  tester.WaitUntilSettled();

  EXPECT_TRUE(tester.IsFulfilled());
  v8::Local<v8::Value> v8_value = tester.Value().V8Value();
  EXPECT_TRUE(v8_value->IsBoolean());
  EXPECT_TRUE(v8_value.As<v8::Boolean>()->Value());
}

}  // namespace blink
