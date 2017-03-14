// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "base/run_loop.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/referrer.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_web_contents_factory.h"
#include "content/public/test/web_contents_tester.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "shared/test/web_contents_test_harness.h"

#include "javascript_dialog.h"
#include "javascript_dialog_client.h"
#include "javascript_dialog_contents_helper.h"
#include "javascript_dialog_factory.h"
#include "javascript_dialog_testing_utils.h"

namespace oxide {

using testing::_;
using testing::StrictMock;

class JavaScriptDialogContentsHelperTest : public WebContentsTestHarness {
 protected:
  JavaScriptDialogContentsHelper* GetJSDialogContentsHelper() const;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager() const;

 private:
  void SetUp() override;
};

JavaScriptDialogContentsHelper*
JavaScriptDialogContentsHelperTest::GetJSDialogContentsHelper() const {
  return JavaScriptDialogContentsHelper::FromWebContents(web_contents());
}

content::JavaScriptDialogManager*
JavaScriptDialogContentsHelperTest::GetJavaScriptDialogManager() const {
  return GetJSDialogContentsHelper();
}

void JavaScriptDialogContentsHelperTest::SetUp() {
  WebContentsTestHarness::SetUp();
  JavaScriptDialogContentsHelper::CreateForWebContents(web_contents());
}

TEST_F(JavaScriptDialogContentsHelperTest, RunJavaScriptDialogWithNoFactory) {
  web_contents()->WasShown();

  int callback_count = 0;
  bool did_suppress_message = false;

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.google.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_ALERT,
      base::string16(), base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count),
      &did_suppress_message);

  EXPECT_TRUE(did_suppress_message);
  EXPECT_EQ(0, callback_count);
}

TEST_F(JavaScriptDialogContentsHelperTest, RunBeforeUnloadDialogWithNoFactory) {
  web_contents()->WasShown();

  int callback_count = 0;
  bool did_suppress_dialog = false;

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunBeforeUnloadDialog(
      web_contents(),
      false,
      false,
      false,
      MakeJavaScriptDialogTestCallback(&callback_count),
      &did_suppress_dialog);

  EXPECT_TRUE(did_suppress_dialog);
  EXPECT_EQ(0, callback_count);
}

namespace {

class MockJavaScriptDialog : public JavaScriptDialog {
 public:
  MockJavaScriptDialog(JavaScriptDialog* sink, JavaScriptDialogClient* client)
      : sink_(sink),
        client_(client),
        weak_ptr_factory_(this) {}

  base::WeakPtr<MockJavaScriptDialog> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  JavaScriptDialogClient* client() const { return client_; }

 private:
  // JavaScriptDialog implementation
  void Show() override { sink_->Show(); }
  void Hide() override { sink_->Hide(); }
  base::string16 GetCurrentPromptText() override {
    sink_->GetCurrentPromptText();
    return base::ASCIIToUTF16("foo");
  }

  JavaScriptDialog* sink_;
  JavaScriptDialogClient* client_;

  base::WeakPtrFactory<MockJavaScriptDialog> weak_ptr_factory_;
};


class MockJavaScriptDialogFactory : public JavaScriptDialogFactory,
                                    public JavaScriptDialog {
 public:
  void Init();

  MockJavaScriptDialog* last_dialog() const { return last_dialog_.get(); }

  // JavaScriptDialogFactory implementation
  MOCK_METHOD2(
      CreateBeforeUnloadDialog,
      std::unique_ptr<JavaScriptDialog>(JavaScriptDialogClient* client,
                                        const GURL& origin_url));
  MOCK_METHOD5(
      CreateJavaScriptDialog,
      std::unique_ptr<JavaScriptDialog>(
          JavaScriptDialogClient* client,
          const GURL& origin_url,
          content::JavaScriptDialogType type,
          const base::string16& message_text,
          const base::string16& default_prompt_text));

  // JavaScriptDialog implementation
  MOCK_METHOD0(Show, void());
  MOCK_METHOD0(Hide, void());
  MOCK_METHOD0(GetCurrentPromptText, base::string16());

  std::unique_ptr<JavaScriptDialog> RealCreateBeforeUnloadDialog(
      JavaScriptDialogClient* client,
      const GURL& origin_url);
  std::unique_ptr<JavaScriptDialog> RealCreateJavaScriptDialog(
      JavaScriptDialogClient* client,
      const GURL& origin_url,
      content::JavaScriptDialogType type,
      const base::string16& message_text,
      const base::string16& default_prompt_text);

 private:
  base::WeakPtr<MockJavaScriptDialog> last_dialog_;
};

void MockJavaScriptDialogFactory::Init() {
  ON_CALL(*this, CreateBeforeUnloadDialog(_,_)).WillByDefault(
      testing::Invoke(
          this,
          &MockJavaScriptDialogFactory::RealCreateBeforeUnloadDialog));
  ON_CALL(*this, CreateJavaScriptDialog(_,_,_,_,_)).WillByDefault(
      testing::Invoke(
          this,
          &MockJavaScriptDialogFactory::RealCreateJavaScriptDialog));
}

std::unique_ptr<JavaScriptDialog>
MockJavaScriptDialogFactory::RealCreateBeforeUnloadDialog(
    JavaScriptDialogClient* client,
    const GURL& origin_url) {
  std::unique_ptr<MockJavaScriptDialog> dialog =
      base::MakeUnique<MockJavaScriptDialog>(this, client);
  last_dialog_ = dialog->GetWeakPtr();
  return std::move(dialog);
}

std::unique_ptr<JavaScriptDialog>
MockJavaScriptDialogFactory::RealCreateJavaScriptDialog(
    JavaScriptDialogClient* client,
    const GURL& origin_url,
    content::JavaScriptDialogType type,
    const base::string16& message_text,
    const base::string16& default_prompt_text) {
  std::unique_ptr<MockJavaScriptDialog> dialog =
      base::MakeUnique<MockJavaScriptDialog>(this, client);
  last_dialog_ = dialog->GetWeakPtr();
  return std::move(dialog);
}

} // namespace

struct JavaScriptDialogContentsHelperTestPRow {
  JavaScriptDialogContentsHelperTestPRow(
      const GURL& origin_url,
      content::JavaScriptDialogType type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      bool success,
      const base::string16& user_input)
      : origin_url(origin_url),
        type(type),
        message_text(message_text),
        default_prompt_text(default_prompt_text),
        success(success),
        user_input(user_input) {}

  GURL origin_url;
  content::JavaScriptDialogType type;
  base::string16 message_text;
  base::string16 default_prompt_text;

  bool success;
  base::string16 user_input;
};

class JavaScriptDialogContentsHelperTestP
    : public JavaScriptDialogContentsHelperTest,
      public testing::WithParamInterface<JavaScriptDialogContentsHelperTestPRow> {
};

INSTANTIATE_TEST_CASE_P(
    Dialogs,
    JavaScriptDialogContentsHelperTestP,
    testing::Values(
        JavaScriptDialogContentsHelperTestPRow(
            GURL("https://www.google.com/"),
            content::JAVASCRIPT_DIALOG_TYPE_ALERT,
            base::ASCIIToUTF16("Foo"), base::string16(),
            true, base::string16()),
        JavaScriptDialogContentsHelperTestPRow(
            GURL("https://www.twitter.com/"),
            content::JAVASCRIPT_DIALOG_TYPE_CONFIRM,
            base::ASCIIToUTF16("Bar"), base::string16(),
            false, base::string16()),
        JavaScriptDialogContentsHelperTestPRow(
            GURL("https://www.google.com/"),
            content::JAVASCRIPT_DIALOG_TYPE_PROMPT,
            base::ASCIIToUTF16("Foo"), base::ASCIIToUTF16("bar"),
            true, base::ASCIIToUTF16("result"))));

TEST_P(JavaScriptDialogContentsHelperTestP, RunDialog) {
  web_contents()->WasShown();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;
  bool did_suppress_dialog = false;

  const auto& params = GetParam();

  EXPECT_CALL(factory,
              CreateJavaScriptDialog(_,
                                     params.origin_url,
                                     params.type,
                                     params.message_text,
                                     params.default_prompt_text));
  EXPECT_CALL(factory, Show());

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      params.origin_url,
      params.type,
      params.message_text,
      params.default_prompt_text,
      MakeJavaScriptDialogTestCallback(&callback_count,
                                       &success,
                                       &user_input),
      &did_suppress_dialog);

  EXPECT_FALSE(did_suppress_dialog);
  EXPECT_EQ(0, callback_count);
  ASSERT_TRUE(factory.last_dialog());
  ASSERT_TRUE(factory.last_dialog()->client());

  EXPECT_CALL(factory, Hide());
  factory.last_dialog()->client()->Close(params.success, params.user_input);

  EXPECT_EQ(1, callback_count);
  EXPECT_EQ(params.success, success);
  EXPECT_EQ(params.user_input, user_input);
  EXPECT_TRUE(factory.last_dialog());

  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(factory.last_dialog());
}

TEST_P(JavaScriptDialogContentsHelperTestP, RunDialogBackground) {
  web_contents()->WasHidden();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;
  bool did_suppress_dialog = false;

  const auto& params = GetParam();

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      params.origin_url,
      params.type,
      params.message_text,
      params.default_prompt_text,
      MakeJavaScriptDialogTestCallback(&callback_count,
                                       &success,
                                       &user_input),
      &did_suppress_dialog);

  EXPECT_EQ(params.type != content::JAVASCRIPT_DIALOG_TYPE_ALERT,
            did_suppress_dialog);
  EXPECT_EQ(params.type == content::JAVASCRIPT_DIALOG_TYPE_ALERT ? 1 : 0,
            callback_count);
  if (params.type == content::JAVASCRIPT_DIALOG_TYPE_ALERT) {
    EXPECT_TRUE(success);
    EXPECT_TRUE(user_input.empty());
  }
  EXPECT_FALSE(factory.last_dialog());

  if (params.type == content::JAVASCRIPT_DIALOG_TYPE_ALERT) {
    EXPECT_CALL(factory,
                CreateJavaScriptDialog(_,
                                       params.origin_url,
                                       content::JAVASCRIPT_DIALOG_TYPE_ALERT,
                                       params.message_text,
                                       params.default_prompt_text));
    EXPECT_CALL(factory, Show());
  }

  web_contents()->WasShown();

  if (params.type == content::JAVASCRIPT_DIALOG_TYPE_ALERT) {
    ASSERT_TRUE(factory.last_dialog());
    ASSERT_TRUE(factory.last_dialog()->client());
    EXPECT_CALL(factory, Hide());
    factory.last_dialog()->client()->Close(true, base::string16());

    EXPECT_TRUE(factory.last_dialog());

    base::RunLoop().RunUntilIdle();
  }

  EXPECT_FALSE(factory.last_dialog());
}

TEST_P(JavaScriptDialogContentsHelperTestP, AutoDismissActiveDialog) {
  web_contents()->WasShown();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;
  bool did_suppress_dialog = false;

  const auto& params = GetParam();

  EXPECT_CALL(factory, CreateJavaScriptDialog(_, _, _, _, _));
  EXPECT_CALL(factory, Show());

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      params.origin_url,
      params.type,
      params.message_text,
      params.default_prompt_text,
      MakeJavaScriptDialogTestCallback(&callback_count,
                                       &success,
                                       &user_input),
      &did_suppress_dialog);

  EXPECT_FALSE(did_suppress_dialog);
  EXPECT_EQ(0, callback_count);
  ASSERT_TRUE(factory.last_dialog());
  ASSERT_TRUE(factory.last_dialog()->client());

  EXPECT_CALL(factory, Hide());
  web_contents()->WasHidden();

  EXPECT_EQ(1, callback_count);
  EXPECT_FALSE(success);
  EXPECT_TRUE(user_input.empty());
  EXPECT_TRUE(factory.last_dialog());

  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(factory.last_dialog());
}

TEST_P(JavaScriptDialogContentsHelperTestP, CancelActiveDialog) {
  web_contents()->WasShown();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;
  bool did_suppress_dialog = false;

  const auto& params = GetParam();

  EXPECT_CALL(factory, CreateJavaScriptDialog(_, _, _, _, _));
  EXPECT_CALL(factory, Show());

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      params.origin_url,
      params.type,
      params.message_text,
      params.default_prompt_text,
      MakeJavaScriptDialogTestCallback(&callback_count,
                                       &success,
                                       &user_input),
      &did_suppress_dialog);

  EXPECT_FALSE(did_suppress_dialog);
  EXPECT_EQ(0, callback_count);
  ASSERT_TRUE(factory.last_dialog());
  ASSERT_TRUE(factory.last_dialog()->client());

  EXPECT_CALL(factory, Hide());
  manager->CancelDialogs(web_contents(), false);

  EXPECT_EQ(1, callback_count);
  EXPECT_FALSE(success);
  EXPECT_TRUE(user_input.empty());
  EXPECT_TRUE(factory.last_dialog());

  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(factory.last_dialog());
}

TEST_P(JavaScriptDialogContentsHelperTestP, SuppressDialogDuringUnload) {
  web_contents()->WasShown();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int before_unload_callback_count = 0;
  bool did_suppress_before_unload_dialog = false;

  const auto& params = GetParam();

  EXPECT_CALL(factory, CreateBeforeUnloadDialog(_, _));
  EXPECT_CALL(factory, Show());

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunBeforeUnloadDialog(
      web_contents(),
      false, false, false,
      MakeJavaScriptDialogTestCallback(&before_unload_callback_count),
      &did_suppress_before_unload_dialog);

  EXPECT_FALSE(did_suppress_before_unload_dialog);
  EXPECT_TRUE(factory.last_dialog());

  int callback_count = 0;
  bool did_suppress_dialog = false;

  manager->RunJavaScriptDialog(
      web_contents(),
      params.origin_url,
      params.type,
      params.message_text,
      params.default_prompt_text,
      MakeJavaScriptDialogTestCallback(&callback_count),
      &did_suppress_dialog);

  EXPECT_TRUE(did_suppress_dialog);
  EXPECT_EQ(0, callback_count);

  EXPECT_EQ(0, before_unload_callback_count);
  EXPECT_TRUE(factory.last_dialog());
}

TEST_F(JavaScriptDialogContentsHelperTest, ReplaceActiveDialog) {
  web_contents()->WasShown();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  EXPECT_CALL(factory, CreateJavaScriptDialog(_, _, _, _, _));
  EXPECT_CALL(factory, Show());

  int callback_count1 = 0;
  bool success1 = false;
  base::string16 user_input1;
  bool did_suppress_dialog1 = false;

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.google.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_ALERT,
      base::ASCIIToUTF16("Bar"),
      base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count1,
                                       &success1,
                                       &user_input1),
      &did_suppress_dialog1);

  EXPECT_FALSE(did_suppress_dialog1);
  EXPECT_EQ(0, callback_count1);
  EXPECT_TRUE(factory.last_dialog());

  EXPECT_CALL(factory, Hide());
  EXPECT_CALL(factory,
              CreateJavaScriptDialog(_,
                                     GURL("https://www.twitter.com/"),
                                     content::JAVASCRIPT_DIALOG_TYPE_CONFIRM,
                                     base::ASCIIToUTF16("Foo"),
                                     base::string16()));
  EXPECT_CALL(factory, Show());

  int callback_count2 = 0;
  bool success2 = false;
  base::string16 user_input2;
  bool did_suppress_dialog2 = false;

  JavaScriptDialog* dialog1 = factory.last_dialog();

  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.twitter.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_CONFIRM,
      base::ASCIIToUTF16("Foo"),
      base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count2,
                                       &success2,
                                       &user_input2),
      &did_suppress_dialog2);

  EXPECT_EQ(1, callback_count1);
  EXPECT_FALSE(success1);
  EXPECT_TRUE(user_input1.empty());
  EXPECT_FALSE(did_suppress_dialog2);
  EXPECT_EQ(0, callback_count2);
  EXPECT_TRUE(factory.last_dialog());
  EXPECT_NE(factory.last_dialog(), dialog1);
}

TEST_F(JavaScriptDialogContentsHelperTest, ReplacePendingDialog) {
  web_contents()->WasHidden();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count1 = 0;
  bool success1 = false;
  base::string16 user_input1;
  bool did_suppress_dialog1 = false;

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.google.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_ALERT,
      base::ASCIIToUTF16("Bar"),
      base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count1,
                                       &success1,
                                       &user_input1),
      &did_suppress_dialog1);

  EXPECT_FALSE(did_suppress_dialog1);
  EXPECT_EQ(1, callback_count1);
  EXPECT_TRUE(success1);
  EXPECT_TRUE(user_input1.empty());
  EXPECT_FALSE(factory.last_dialog());

  int callback_count2 = 0;
  bool success2 = false;
  base::string16 user_input2;
  bool did_suppress_dialog2 = false;

  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.twitter.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_ALERT,
      base::ASCIIToUTF16("Foo"),
      base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count2,
                                       &success2,
                                       &user_input2),
      &did_suppress_dialog2);

  EXPECT_FALSE(did_suppress_dialog2);
  EXPECT_EQ(1, callback_count2);
  EXPECT_TRUE(success2);
  EXPECT_TRUE(user_input2.empty());
  EXPECT_FALSE(factory.last_dialog());

  EXPECT_CALL(factory,
              CreateJavaScriptDialog(_,
                                     GURL("https://www.twitter.com/"),
                                     content::JAVASCRIPT_DIALOG_TYPE_ALERT,
                                     base::ASCIIToUTF16("Foo"),
                                     base::string16()));
  EXPECT_CALL(factory, Show());

  web_contents()->WasShown();
  EXPECT_TRUE(factory.last_dialog());
}

TEST_F(JavaScriptDialogContentsHelperTest, CancelPendingDialog) {
  web_contents()->WasHidden();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;
  bool did_suppress_dialog = false;

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.google.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_ALERT,
      base::ASCIIToUTF16("Bar"),
      base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count,
                                       &success,
                                       &user_input),
      &did_suppress_dialog);

  EXPECT_FALSE(did_suppress_dialog);
  EXPECT_EQ(1, callback_count);
  EXPECT_TRUE(success);
  EXPECT_TRUE(user_input.empty());
  EXPECT_FALSE(factory.last_dialog());

  manager->CancelDialogs(web_contents(), false);

  web_contents()->WasShown();
  EXPECT_FALSE(factory.last_dialog());
}

TEST_F(JavaScriptDialogContentsHelperTest,
       ConfirmDialogShouldntReplacePendingAlertDialog) {
  web_contents()->WasHidden();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count1 = 0;
  bool success1 = false;
  base::string16 user_input1;
  bool did_suppress_dialog1 = false;

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.google.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_ALERT,
      base::ASCIIToUTF16("Bar"),
      base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count1,
                                       &success1,
                                       &user_input1),
      &did_suppress_dialog1);

  EXPECT_FALSE(did_suppress_dialog1);
  EXPECT_EQ(1, callback_count1);
  EXPECT_TRUE(success1);
  EXPECT_TRUE(user_input1.empty());
  EXPECT_FALSE(factory.last_dialog());

  int callback_count2 = 0;
  bool did_suppress_dialog2 = false;

  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.twitter.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_CONFIRM,
      base::ASCIIToUTF16("Foo"),
      base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count2),
      &did_suppress_dialog2);

  EXPECT_TRUE(did_suppress_dialog2);
  EXPECT_EQ(0, callback_count2);
  EXPECT_FALSE(factory.last_dialog());

  EXPECT_CALL(factory,
              CreateJavaScriptDialog(_,
                                     GURL("https://www.google.com/"),
                                     content::JAVASCRIPT_DIALOG_TYPE_ALERT,
                                     base::ASCIIToUTF16("Bar"),
                                     base::string16()));
  EXPECT_CALL(factory, Show());

  web_contents()->WasShown();
  EXPECT_TRUE(factory.last_dialog());
}

struct JavaScriptDialogContentsHelperBeforeUnloadTestPRow {
  JavaScriptDialogContentsHelperBeforeUnloadTestPRow(
      const GURL& origin_url,
      bool is_renderer_initiated,
      bool has_user_gesture,
      bool success)
      : origin_url(origin_url),
        is_renderer_initiated(is_renderer_initiated),
        has_user_gesture(has_user_gesture),
        success(success) {}

  GURL origin_url;
  bool is_renderer_initiated;
  bool has_user_gesture;

  bool success;
};

class JavaScriptDialogContentsHelperBeforeUnloadTestP
    : public JavaScriptDialogContentsHelperTest,
      public testing::WithParamInterface<JavaScriptDialogContentsHelperBeforeUnloadTestPRow> {
};

INSTANTIATE_TEST_CASE_P(
    BeforeUnloadDialogs,
    JavaScriptDialogContentsHelperBeforeUnloadTestP,
    testing::Values(
        JavaScriptDialogContentsHelperBeforeUnloadTestPRow(
            GURL("https://www.google.com/"),
            false, false, true),
        JavaScriptDialogContentsHelperBeforeUnloadTestPRow(
            GURL("https://www.google.com/"),
            true, false, false),
        JavaScriptDialogContentsHelperBeforeUnloadTestPRow(
            GURL("https://www.facebook.com/"),
            true, true, false)));

TEST_P(JavaScriptDialogContentsHelperBeforeUnloadTestP, RunBeforeUnload) {
  const auto& params = GetParam();

  web_contents()->WasShown();
  content::WebContentsTester::For(web_contents())->NavigateAndCommit(
      params.origin_url);

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;
  bool did_suppress_dialog = false;

  bool will_suppress = params.is_renderer_initiated && !params.has_user_gesture;

  if (!will_suppress) {
    EXPECT_CALL(factory, CreateBeforeUnloadDialog(_, params.origin_url));
    EXPECT_CALL(factory, Show());
  }

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunBeforeUnloadDialog(
      web_contents(),
      false,
      params.is_renderer_initiated,
      params.has_user_gesture,
      MakeJavaScriptDialogTestCallback(&callback_count, &success, &user_input),
      &did_suppress_dialog);

  EXPECT_EQ(will_suppress, did_suppress_dialog);
  EXPECT_EQ(0, callback_count);
  if (!did_suppress_dialog) {
    ASSERT_TRUE(factory.last_dialog());
    ASSERT_TRUE(factory.last_dialog()->client());

    EXPECT_CALL(factory, Hide());
    factory.last_dialog()->client()->Close(params.success, base::string16());

    EXPECT_EQ(1, callback_count);
    EXPECT_EQ(params.success, success);
    EXPECT_TRUE(user_input.empty());
    EXPECT_TRUE(factory.last_dialog());

    base::RunLoop().RunUntilIdle();
  }

  EXPECT_FALSE(factory.last_dialog());
}

TEST_P(JavaScriptDialogContentsHelperBeforeUnloadTestP,
       RunBeforeUnloadBackground) {
  const auto& params = GetParam();

  web_contents()->WasHidden();
  content::WebContentsTester::For(web_contents())->NavigateAndCommit(
      params.origin_url);

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;
  bool did_suppress_dialog = false;

  bool will_suppress = params.is_renderer_initiated && !params.has_user_gesture;
  bool will_block = params.is_renderer_initiated && params.has_user_gesture;

  if (!will_suppress && !will_block) {
    EXPECT_CALL(factory, CreateBeforeUnloadDialog(_, _));
    EXPECT_CALL(factory, Show());
  }

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunBeforeUnloadDialog(
      web_contents(),
      false,
      params.is_renderer_initiated,
      params.has_user_gesture,
      MakeJavaScriptDialogTestCallback(&callback_count, &success, &user_input),
      &did_suppress_dialog);

  EXPECT_EQ(will_suppress, did_suppress_dialog);
  EXPECT_EQ(will_block ? 1 : 0, callback_count);
  if (will_block) {
    EXPECT_FALSE(success);
    EXPECT_TRUE(user_input.empty());
  }
}

TEST_F(JavaScriptDialogContentsHelperTest,
       BeforeUnloadDialogShouldDismissActiveDialogs) {
  web_contents()->WasShown();
  content::WebContentsTester::For(web_contents())->NavigateAndCommit(
      GURL("https://www.google.com/"));

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  EXPECT_CALL(factory, CreateJavaScriptDialog(_, _, _, _, _));
  EXPECT_CALL(factory, Show());

  int callback_count1 = 0;
  bool success1 = false;
  base::string16 user_input1;
  bool did_suppress_dialog1 = false;

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.google.com/a"),
      content::JAVASCRIPT_DIALOG_TYPE_CONFIRM,
      base::string16(), base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count1,
                                       &success1,
                                       &user_input1),
      &did_suppress_dialog1);

  EXPECT_FALSE(did_suppress_dialog1);
  EXPECT_EQ(0, callback_count1);
  EXPECT_TRUE(factory.last_dialog());

  EXPECT_CALL(factory, Hide());
  EXPECT_CALL(factory,
              CreateBeforeUnloadDialog(_,
                                       GURL("https://www.google.com/")));
  EXPECT_CALL(factory, Show());

  JavaScriptDialog* dialog1 = factory.last_dialog();

  int callback_count2 = 0;
  bool did_suppress_dialog2 = false;

  manager->RunBeforeUnloadDialog(
      web_contents(),
      false, false, false,
      MakeJavaScriptDialogTestCallback(&callback_count2),
      &did_suppress_dialog2);

  EXPECT_FALSE(did_suppress_dialog2);
  EXPECT_EQ(0, callback_count2);
  EXPECT_TRUE(factory.last_dialog());
  EXPECT_NE(dialog1, factory.last_dialog());
  EXPECT_EQ(1, callback_count1);
  EXPECT_FALSE(success1);
  EXPECT_TRUE(user_input1.empty());
}

TEST_F(JavaScriptDialogContentsHelperTest,
       BeforeUnloadDialogShouldDismissPendingDialogs) {
  web_contents()->WasHidden();
  content::WebContentsTester::For(web_contents())->NavigateAndCommit(
      GURL("https://www.google.com/"));

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count1 = 0;
  bool success1 = false;
  base::string16 user_input1;
  bool did_suppress_dialog1 = false;

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.google.com/a"),
      content::JAVASCRIPT_DIALOG_TYPE_ALERT,
      base::string16(), base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count1,
                                       &success1,
                                       &user_input1),
      &did_suppress_dialog1);

  EXPECT_FALSE(did_suppress_dialog1);
  EXPECT_EQ(1, callback_count1);
  EXPECT_TRUE(success1);
  EXPECT_TRUE(user_input1.empty());
  EXPECT_FALSE(factory.last_dialog());

  EXPECT_CALL(factory,
              CreateBeforeUnloadDialog(_,
                                       GURL("https://www.google.com/")));
  EXPECT_CALL(factory, Show());

  int callback_count2 = 0;
  bool did_suppress_dialog2 = false;

  manager->RunBeforeUnloadDialog(
      web_contents(),
      false, false, false,
      MakeJavaScriptDialogTestCallback(&callback_count2),
      &did_suppress_dialog2);

  EXPECT_FALSE(did_suppress_dialog2);
  EXPECT_EQ(0, callback_count2);
  EXPECT_TRUE(factory.last_dialog());
  EXPECT_EQ(1, callback_count1);
}

class JavaScriptDialogContentsHelperHandleTestP
    : public JavaScriptDialogContentsHelperTest,
      public testing::WithParamInterface<bool> {};

INSTANTIATE_TEST_CASE_P(, JavaScriptDialogContentsHelperHandleTestP,
                        testing::Values(true, false));

TEST_P(JavaScriptDialogContentsHelperHandleTestP,
       HandleJavaScriptDialogNoActive) {
  base::string16 prompt_override = base::ASCIIToUTF16("bar");
  EXPECT_FALSE(
      GetJavaScriptDialogManager()->HandleJavaScriptDialog(
          web_contents(),
          GetParam(), GetParam() ? &prompt_override : nullptr));
}

TEST_P(JavaScriptDialogContentsHelperHandleTestP, HandleJavaScriptDialog) {
  web_contents()->WasShown();

  StrictMock<MockJavaScriptDialogFactory> factory;
  factory.Init();
  JavaScriptDialogContentsHelper* helper = GetJSDialogContentsHelper();
  helper->set_factory(&factory);

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;
  bool did_suppress_dialog = false;

  EXPECT_CALL(factory, CreateJavaScriptDialog(_, _, _, _, _));
  EXPECT_CALL(factory, Show());

  content::JavaScriptDialogManager* manager = GetJavaScriptDialogManager();
  manager->RunJavaScriptDialog(
      web_contents(),
      GURL("https://www.google.com/"),
      content::JAVASCRIPT_DIALOG_TYPE_PROMPT,
      base::string16(),
      base::string16(),
      MakeJavaScriptDialogTestCallback(&callback_count,
                                       &success,
                                       &user_input),
      &did_suppress_dialog);

  EXPECT_FALSE(did_suppress_dialog);
  EXPECT_EQ(0, callback_count);
  EXPECT_TRUE(factory.last_dialog());

  if (!GetParam()) {
    EXPECT_CALL(factory, GetCurrentPromptText());
  }
  EXPECT_CALL(factory, Hide());

  base::string16 prompt_override = base::ASCIIToUTF16("bar");
  EXPECT_TRUE(
      manager->HandleJavaScriptDialog(
          web_contents(),
          GetParam(),
          GetParam() ? &prompt_override : nullptr));

  EXPECT_EQ(1, callback_count);
  EXPECT_EQ(GetParam(), success);
  EXPECT_EQ(GetParam() ? prompt_override : base::ASCIIToUTF16("foo"),
            user_input);
  EXPECT_TRUE(factory.last_dialog());

  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(factory.last_dialog());
}

} // namespace oxide
