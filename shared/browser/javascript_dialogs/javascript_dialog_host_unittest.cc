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
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_web_contents_factory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

#include "shared/test/test_browser_thread_bundle.h"

#include "javascript_dialog.h"
#include "javascript_dialog_contents_helper.h"
#include "javascript_dialog_factory.h"
#include "javascript_dialog_host.h"
#include "javascript_dialog_testing_utils.h"

namespace oxide {

using testing::_;

class JavaScriptDialogHostTest : public testing::Test {
 protected:
  content::WebContents* web_contents() const { return web_contents_; }

  JavaScriptDialogContentsHelper* GetJSDialogContentsHelper() const;

 private:
  void SetUp() override;

  TestBrowserThreadBundle browser_thread_bundle_;
  content::TestBrowserContext browser_context_;
  content::TestWebContentsFactory web_contents_factory_;

  content::WebContents* web_contents_ = nullptr;
};

JavaScriptDialogContentsHelper*
JavaScriptDialogHostTest::GetJSDialogContentsHelper() const {
  return JavaScriptDialogContentsHelper::FromWebContents(web_contents_);
}

void JavaScriptDialogHostTest::SetUp() {
  web_contents_ = web_contents_factory_.CreateWebContents(&browser_context_);
  JavaScriptDialogContentsHelper::CreateForWebContents(web_contents_);
}

struct JavaScriptDialogHostConstructTestRow {
  JavaScriptDialogHostConstructTestRow(
      const GURL& origin_url,
      bool is_before_unload_dialog,
      content::JavaScriptDialogType type,
      const base::string16& message_text,
      const base::string16& default_prompt_text)
      : origin_url(origin_url),
        is_before_unload_dialog(is_before_unload_dialog),
        type(type),
        message_text(message_text),
        default_prompt_text(default_prompt_text) {}

  GURL origin_url;
  bool is_before_unload_dialog;
  content::JavaScriptDialogType type;
  base::string16 message_text;
  base::string16 default_prompt_text;
};

class JavaScriptDialogHostConstructTest
    : public JavaScriptDialogHostTest,
      public testing::WithParamInterface<JavaScriptDialogHostConstructTestRow> {
 protected:
  std::unique_ptr<JavaScriptDialogHost> CreateJavaScriptDialogHost(
      const content::JavaScriptDialogManager::DialogClosedCallback& callback) {
    return base::MakeUnique<JavaScriptDialogHost>(
        GetJSDialogContentsHelper(),
        GetParam().origin_url,
        GetParam().is_before_unload_dialog,
        GetParam().type,
        GetParam().message_text,
        GetParam().default_prompt_text,
        callback);
  }
};

INSTANTIATE_TEST_CASE_P(
    Dialogs,
    JavaScriptDialogHostConstructTest,
    testing::Values(
        JavaScriptDialogHostConstructTestRow(
            GURL("https://www.google.com/"),
            false,
            content::JAVASCRIPT_DIALOG_TYPE_ALERT,
            base::ASCIIToUTF16("Foo"), base::string16()),
        JavaScriptDialogHostConstructTestRow(
            GURL("https://www.twitter.com/"),
            false,
            content::JAVASCRIPT_DIALOG_TYPE_CONFIRM,
            base::ASCIIToUTF16("Bar"), base::string16()),
        JavaScriptDialogHostConstructTestRow(
            GURL("https://www.google.com/"),
            false,
            content::JAVASCRIPT_DIALOG_TYPE_PROMPT,
            base::ASCIIToUTF16("Foo"), base::ASCIIToUTF16("bar")),
        JavaScriptDialogHostConstructTestRow(
            GURL("https://www.google.com/"),
            true,
            content::JAVASCRIPT_DIALOG_TYPE_CONFIRM,
            base::string16(), base::string16())));

TEST_P(JavaScriptDialogHostConstructTest, NoFactory) {
  const auto& params = GetParam();

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;

  std::unique_ptr<JavaScriptDialogHost> host =
      CreateJavaScriptDialogHost(
          MakeJavaScriptDialogTestCallback(&callback_count,
                                           &success,
                                           &user_input));
  EXPECT_EQ(0, callback_count);

  host->Show();

  EXPECT_EQ(1, callback_count);
  EXPECT_EQ(params.is_before_unload_dialog, success);
  EXPECT_TRUE(user_input.empty());
}

namespace {

class MockJavaScriptDialogFactory : public JavaScriptDialogFactory {
 public:
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
};

}

TEST_P(JavaScriptDialogHostConstructTest, MockFactory) {
  const auto& params = GetParam();

  int callback_count = 0;
  bool success = false;
  base::string16 user_input;

  MockJavaScriptDialogFactory factory;
  GetJSDialogContentsHelper()->set_factory(&factory);

  if (params.is_before_unload_dialog) {
    EXPECT_CALL(factory,
                CreateBeforeUnloadDialog(_, params.origin_url));
  } else {
    EXPECT_CALL(factory,
                CreateJavaScriptDialog(_,
                                       params.origin_url,
                                       params.type,
                                       params.message_text,
                                       params.default_prompt_text));
  }

  std::unique_ptr<JavaScriptDialogHost> host =
      CreateJavaScriptDialogHost(
          MakeJavaScriptDialogTestCallback(&callback_count,
                                           &success,
                                           &user_input));
  EXPECT_EQ(0, callback_count);

  host->Show();

  EXPECT_EQ(1, callback_count);
  EXPECT_EQ(params.is_before_unload_dialog, success);
  EXPECT_TRUE(user_input.empty());
}

namespace {

class FakeJavaScriptDialogFactory : public JavaScriptDialogFactory,
                                    public JavaScriptDialog {
 public:
  MOCK_METHOD0(Show, void());
  MOCK_METHOD0(Hide, void());
  MOCK_METHOD0(GetCurrentPromptText, base::string16());

 private:
  // JavaScriptDialogFactory implementation
  std::unique_ptr<JavaScriptDialog> CreateBeforeUnloadDialog(
      JavaScriptDialogClient* client,
      const GURL& origin_url) override;
  std::unique_ptr<JavaScriptDialog> CreateJavaScriptDialog(
      JavaScriptDialogClient* client,
      const GURL& origin_url,
      content::JavaScriptDialogType type,
      const base::string16& message_text,
      const base::string16& default_prompt_text) override;
};

class MockJavaScriptDialog : public JavaScriptDialog {
 public:
  MockJavaScriptDialog(JavaScriptDialog* sink)
      : sink_(sink) {}

 private:
  // JavaScriptDialog implementation
  void Show() override { sink_->Show(); }
  void Hide() override { sink_->Hide(); }
  base::string16 GetCurrentPromptText() override {
    sink_->GetCurrentPromptText();
    return base::ASCIIToUTF16("bar");
  }

  JavaScriptDialog* sink_;
};

std::unique_ptr<JavaScriptDialog>
FakeJavaScriptDialogFactory::CreateBeforeUnloadDialog(
    JavaScriptDialogClient* client,
    const GURL& origin_url) {
  return base::MakeUnique<MockJavaScriptDialog>(this);
}

std::unique_ptr<JavaScriptDialog>
FakeJavaScriptDialogFactory::CreateJavaScriptDialog(
    JavaScriptDialogClient* client,
    const GURL& origin_url,
    content::JavaScriptDialogType type,
    const base::string16& message_text,
    const base::string16& default_prompt_text) {
  return base::MakeUnique<MockJavaScriptDialog>(this);
}

} // namespace

TEST_F(JavaScriptDialogHostTest, Show) {
  int callback_count = 0;

  FakeJavaScriptDialogFactory factory;
  GetJSDialogContentsHelper()->set_factory(&factory);

  std::unique_ptr<JavaScriptDialogHost> host =
      base::MakeUnique<JavaScriptDialogHost>(
          GetJSDialogContentsHelper(),
          GURL("https://www.google.com/"),
          false, content::JAVASCRIPT_DIALOG_TYPE_ALERT,
          base::string16(), base::string16(),
          MakeJavaScriptDialogTestCallback(&callback_count));

  EXPECT_CALL(factory, Show());

  host->Show();

  EXPECT_EQ(0, callback_count);
}

TEST_F(JavaScriptDialogHostTest, Close) {
  int callback_count = 0;
  bool success = false;
  base::string16 user_input;

  FakeJavaScriptDialogFactory factory;
  GetJSDialogContentsHelper()->set_factory(&factory);

  std::unique_ptr<JavaScriptDialogHost> host =
      base::MakeUnique<JavaScriptDialogHost>(
          GetJSDialogContentsHelper(),
          GURL("https://www.google.com/"),
          false, content::JAVASCRIPT_DIALOG_TYPE_PROMPT,
          base::string16(), base::string16(),
          MakeJavaScriptDialogTestCallback(&callback_count,
                                           &success,
                                           &user_input));

  EXPECT_CALL(factory, Show());
  host->Show();

  EXPECT_CALL(factory, Hide());
  static_cast<JavaScriptDialogClient*>(host.get())
      ->Close(true, base::ASCIIToUTF16("bar"));

  EXPECT_EQ(1, callback_count);
  EXPECT_TRUE(success);
  EXPECT_EQ(user_input, base::ASCIIToUTF16("bar"));
}

TEST_F(JavaScriptDialogHostTest, Dismiss) {
  int callback_count = 0;
  bool success = false;
  base::string16 user_input;

  FakeJavaScriptDialogFactory factory;
  GetJSDialogContentsHelper()->set_factory(&factory);

  std::unique_ptr<JavaScriptDialogHost> host =
      base::MakeUnique<JavaScriptDialogHost>(
          GetJSDialogContentsHelper(),
          GURL("https://www.google.com/"),
          false, content::JAVASCRIPT_DIALOG_TYPE_CONFIRM,
          base::string16(), base::string16(),
          MakeJavaScriptDialogTestCallback(&callback_count,
                                           &success,
                                           &user_input));

  EXPECT_CALL(factory, Show());
  host->Show();

  EXPECT_CALL(factory, Hide());
  host->Dismiss();

  EXPECT_EQ(1, callback_count);
  EXPECT_FALSE(success);
  EXPECT_TRUE(user_input.empty());
}

TEST_F(JavaScriptDialogHostTest, HandleWithOverride) {
  int callback_count = 0;
  bool success = false;
  base::string16 user_input;

  FakeJavaScriptDialogFactory factory;
  GetJSDialogContentsHelper()->set_factory(&factory);

  std::unique_ptr<JavaScriptDialogHost> host =
      base::MakeUnique<JavaScriptDialogHost>(
          GetJSDialogContentsHelper(),
          GURL("https://www.google.com/"),
          false, content::JAVASCRIPT_DIALOG_TYPE_PROMPT,
          base::string16(), base::string16(),
          MakeJavaScriptDialogTestCallback(&callback_count,
                                           &success,
                                           &user_input));

  EXPECT_CALL(factory, Show());
  host->Show();

  EXPECT_CALL(factory, Hide());
  base::string16 prompt_override = base::ASCIIToUTF16("foo");
  host->Handle(true, &prompt_override);

  EXPECT_EQ(1, callback_count);
  EXPECT_TRUE(success);
  EXPECT_EQ(user_input, base::ASCIIToUTF16("foo"));
}

TEST_F(JavaScriptDialogHostTest, HandleNoOverride) {
  int callback_count = 0;
  bool success = false;
  base::string16 user_input;

  FakeJavaScriptDialogFactory factory;
  GetJSDialogContentsHelper()->set_factory(&factory);

  std::unique_ptr<JavaScriptDialogHost> host =
      base::MakeUnique<JavaScriptDialogHost>(
          GetJSDialogContentsHelper(),
          GURL("https://www.google.com/"),
          false, content::JAVASCRIPT_DIALOG_TYPE_PROMPT,
          base::string16(), base::string16(),
          MakeJavaScriptDialogTestCallback(&callback_count,
                                           &success,
                                           &user_input));

  EXPECT_CALL(factory, Show());
  host->Show();

  EXPECT_CALL(factory, Hide());
  host->Handle(false, nullptr);

  EXPECT_EQ(1, callback_count);
  EXPECT_FALSE(success);
  EXPECT_EQ(user_input, base::ASCIIToUTF16("bar"));
}

} // namespace oxide
