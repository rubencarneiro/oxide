// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef OXIDE_Q_WEB_PREFERENCES
#define OXIDE_Q_WEB_PREFERENCES

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

class OxideQWebPreferencesPrivate;

class Q_DECL_EXPORT OxideQWebPreferences : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString standardFontFamily READ standardFontFamily WRITE setStandardFontFamily NOTIFY standardFontFamilyChanged)
  Q_PROPERTY(QString fixedFontFamily READ fixedFontFamily WRITE setFixedFontFamily NOTIFY fixedFontFamilyChanged)
  Q_PROPERTY(QString serifFontFamily READ serifFontFamily WRITE setSerifFontFamily NOTIFY serifFontFamilyChanged)
  Q_PROPERTY(QString sanSerifFontFamily READ sansSerifFontFamily WRITE setSansSerifFontFamily NOTIFY sansSerifFontFamilyChanged)

  Q_PROPERTY(bool remoteFontsEnabled READ remoteFontsEnabled WRITE setRemoteFontsEnabled NOTIFY remoteFontsEnabledChanged)

  Q_PROPERTY(QString defaultEncoding READ defaultEncoding WRITE setDefaultEncoding NOTIFY defaultEncodingChanged)

  Q_PROPERTY(unsigned defaultFontSize READ defaultFontSize WRITE setDefaultFontSize NOTIFY defaultFontSizeChanged)
  Q_PROPERTY(unsigned defaultFixedFontSize READ defaultFixedFontSize WRITE setDefaultFixedFontSize NOTIFY defaultFixedFontSizeChanged)
  Q_PROPERTY(unsigned minimumFontSize READ minimumFontSize WRITE setMinimumFontSize NOTIFY minimumFontSizeChanged)

  Q_PROPERTY(bool javascriptEnabled READ javascriptEnabled WRITE setJavascriptEnabled NOTIFY javascriptEnabledChanged)
  Q_PROPERTY(bool webSecurityEnabled READ webSecurityEnabled WRITE setWebSecurityEnabled NOTIFY webSecurityEnabledChanged)
  Q_PROPERTY(bool popupBlockerEnabled READ popupBlockerEnabled WRITE setPopupBlockerEnabled NOTIFY popupBlockerEnabledChanged)
  Q_PROPERTY(bool allowScriptsToCloseWindows READ allowScriptsToCloseWindows WRITE setAllowScriptsToCloseWindows NOTIFY allowScriptsToCloseWindowsChanged)
  Q_PROPERTY(bool javascriptCanAccessClipboard READ javascriptCanAccessClipboard WRITE setJavascriptCanAccessClipboard NOTIFY javascriptCanAccessClipboardChanged)

  Q_PROPERTY(bool hyperlinkAuditingEnabled READ hyperlinkAuditingEnabled WRITE setHyperlinkAuditingEnabled NOTIFY hyperlinkAuditingEnabledChanged)
  Q_PROPERTY(bool allowUniversalAccessFromFileUrls READ allowUniversalAccessFromFileUrls WRITE setAllowUniversalAccessFromFileUrls NOTIFY allowUniversalAccessFromFileUrlsChanged)
  Q_PROPERTY(bool allowFileAccessFromFileUrls READ allowFileAccessFromFileUrls WRITE setAllowFileAccessFromFileUrls NOTIFY allowFileAccessFromFileUrlsChanged)
  Q_PROPERTY(bool canDisplayInsecureContent READ canDisplayInsecureContent WRITE setCanDisplayInsecureContent NOTIFY canDisplayInsecureContentChanged)
  Q_PROPERTY(bool canRunInsecureContent READ canRunInsecureContent WRITE setCanRunInsecureContent NOTIFY canRunInsecureContentChanged)
  Q_PROPERTY(bool passwordEchoEnabled READ passwordEchoEnabled WRITE setPasswordEchoEnabled NOTIFY passwordEchoEnabledChanged)

  Q_PROPERTY(bool loadsImagesAutomatically READ loadsImagesAutomatically WRITE setLoadsImagesAutomatically NOTIFY loadsImagesAutomaticallyChanged)
  Q_PROPERTY(bool shrinksStandaloneImagesToFit READ shrinksStandaloneImagesToFit WRITE setShrinksStandaloneImagesToFit NOTIFY shrinksStandaloneImagesToFitChanged)

  Q_PROPERTY(bool textAreasAreResizable READ textAreasAreResizable WRITE setTextAreasAreResizable NOTIFY textAreasAreResizableChanged)
  
  Q_PROPERTY(bool localStorageEnabled READ localStorageEnabled WRITE setLocalStorageEnabled NOTIFY localStorageEnabledChanged)
  Q_PROPERTY(bool databasesEnabled READ databasesEnabled WRITE setDatabasesEnabled NOTIFY databasesEnabledChanged)
  Q_PROPERTY(bool appCacheEnabled READ appCacheEnabled WRITE setAppCacheEnabled NOTIFY appCacheEnabledChanged)
  Q_PROPERTY(bool fullscreenEnabled READ fullscreenEnabled WRITE setFullscreenEnabled NOTIFY fullscreenEnabledChanged)

  Q_PROPERTY(bool tabsToLinks READ tabsToLinks WRITE setTabsToLinks NOTIFY tabsToLinksChanged)
  Q_PROPERTY(bool caretBrowsingEnabled READ caretBrowsingEnabled WRITE setCaretBrowsingEnabled NOTIFY caretBrowsingEnabledChanged)

  Q_PROPERTY(bool smoothScrollingEnabled READ smoothScrollingEnabled WRITE setSmoothScrollingEnabled NOTIFY smoothScrollingEnabledChanged)
  Q_PROPERTY(bool touchEnabled READ touchEnabled WRITE setTouchEnabled NOTIFY touchEnabledChanged)
  Q_PROPERTY(bool supportsMultipleWindows READ supportsMultipleWindows WRITE setSupportsMultipleWindows NOTIFY supportsMultipleWindowsChanged)

  Q_DECLARE_PRIVATE(OxideQWebPreferences)
  Q_DISABLE_COPY(OxideQWebPreferences)

 public:
  virtual ~OxideQWebPreferences();
  OxideQWebPreferences(QObject* parent = NULL);

  QString standardFontFamily() const;
  void setStandardFontFamily(const QString& font);
  QString fixedFontFamily() const;
  void setFixedFontFamily(const QString& font);
  QString serifFontFamily() const;
  void setSerifFontFamily(const QString& font);
  QString sansSerifFontFamily() const;
  void setSansSerifFontFamily(const QString& font);

  bool remoteFontsEnabled() const;
  void setRemoteFontsEnabled(bool enabled);

  QString defaultEncoding() const;
  void setDefaultEncoding(const QString& encoding);

  unsigned defaultFontSize() const;
  void setDefaultFontSize(unsigned size);
  unsigned defaultFixedFontSize() const;
  void setDefaultFixedFontSize(unsigned size);
  unsigned minimumFontSize() const;
  void setMinimumFontSize(unsigned size);

  bool javascriptEnabled() const;
  void setJavascriptEnabled(bool enabled);
  bool webSecurityEnabled() const;
  void setWebSecurityEnabled(bool enabled);
  bool popupBlockerEnabled() const;
  void setPopupBlockerEnabled(bool enabled);
  bool allowScriptsToCloseWindows() const;
  void setAllowScriptsToCloseWindows(bool allow);
  bool javascriptCanAccessClipboard() const;
  void setJavascriptCanAccessClipboard(bool allow);

  bool hyperlinkAuditingEnabled() const;
  void setHyperlinkAuditingEnabled(bool enabled);
  bool allowUniversalAccessFromFileUrls() const;
  void setAllowUniversalAccessFromFileUrls(bool allow);
  bool allowFileAccessFromFileUrls() const;
  void setAllowFileAccessFromFileUrls(bool allow);
  bool canDisplayInsecureContent() const;
  void setCanDisplayInsecureContent(bool allow);
  bool canRunInsecureContent() const;
  void setCanRunInsecureContent(bool allow);
  bool passwordEchoEnabled() const;
  void setPasswordEchoEnabled(bool enabled);

  bool loadsImagesAutomatically() const;
  void setLoadsImagesAutomatically(bool enabled);
  bool shrinksStandaloneImagesToFit() const;
  void setShrinksStandaloneImagesToFit(bool enabled);

  bool textAreasAreResizable() const;
  void setTextAreasAreResizable(bool enabled);

  bool localStorageEnabled() const;
  void setLocalStorageEnabled(bool enabled);
  bool databasesEnabled() const;
  void setDatabasesEnabled(bool enabled);
  bool appCacheEnabled() const;
  void setAppCacheEnabled(bool enabled);
  bool fullscreenEnabled() const;
  void setFullscreenEnabled(bool enabled);

  bool tabsToLinks() const;
  void setTabsToLinks(bool enabled);
  bool caretBrowsingEnabled() const;
  void setCaretBrowsingEnabled(bool enabled);

  bool smoothScrollingEnabled() const;
  void setSmoothScrollingEnabled(bool enabled);
  bool touchEnabled() const;
  void setTouchEnabled(bool enabled);
  bool supportsMultipleWindows() const;
  void setSupportsMultipleWindows(bool enabled);

 Q_SIGNALS:
  void standardFontFamilyChanged();
  void fixedFontFamilyChanged();
  void serifFontFamilyChanged();
  void sansSerifFontFamilyChanged();

  void remoteFontsEnabledChanged();

  void defaultEncodingChanged();

  void defaultFontSizeChanged();
  void defaultFixedFontSizeChanged();
  void minimumFontSizeChanged();

  void javascriptEnabledChanged();
  void webSecurityEnabledChanged();
  void popupBlockerEnabledChanged();
  void allowScriptsToCloseWindowsChanged();
  void javascriptCanAccessClipboardChanged();

  void hyperlinkAuditingEnabledChanged();
  void allowUniversalAccessFromFileUrlsChanged();
  void allowFileAccessFromFileUrlsChanged();
  void canDisplayInsecureContentChanged();
  void canRunInsecureContentChanged();
  void passwordEchoEnabledChanged();

  void loadsImagesAutomaticallyChanged();
  void shrinksStandaloneImagesToFitChanged();

  void textAreasAreResizableChanged();

  void localStorageEnabledChanged();
  void databasesEnabledChanged();
  void appCacheEnabledChanged();
  void fullscreenEnabledChanged();

  void tabsToLinksChanged();
  void caretBrowsingEnabledChanged();

  void smoothScrollingEnabledChanged();
  void touchEnabledChanged();
  void supportsMultipleWindowsChanged();

 private:
  QScopedPointer<OxideQWebPreferencesPrivate> d_ptr;
};

#endif // OXIDE_Q_WEB_PREFERENCES
