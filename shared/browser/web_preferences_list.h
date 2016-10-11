#define WEB_PREF_DEFAULT(...) __VA_ARGS__
WEB_PREF(content::ScriptFontFamilyMap,
         standard_font_family_map,
         WEB_PREF_DEFAULT({{content::kCommonScript, base::UTF8ToUTF16("Times New Roman")}}))
WEB_PREF(content::ScriptFontFamilyMap,
         fixed_font_family_map,
         WEB_PREF_DEFAULT({{content::kCommonScript, base::UTF8ToUTF16("Courier New")}}))
WEB_PREF(content::ScriptFontFamilyMap,
         serif_font_family_map,
         WEB_PREF_DEFAULT({{content::kCommonScript, base::UTF8ToUTF16("Times New Roman")}}))
WEB_PREF(content::ScriptFontFamilyMap,
         sans_serif_font_family_map,
         WEB_PREF_DEFAULT({{content::kCommonScript, base::UTF8ToUTF16("Arial")}}))
WEB_PREF(int, default_font_size, 16)
WEB_PREF(int, default_fixed_font_size, 13)
WEB_PREF(int, minimum_font_size, 0)
WEB_PREF(std::string, default_encoding, "ISO-8859-1")
WEB_PREF(bool, remote_fonts_enabled, true)
WEB_PREF(bool, javascript_enabled, true)
WEB_PREF(bool, allow_scripts_to_close_windows, false)
WEB_PREF(bool, javascript_can_access_clipboard, false)
WEB_PREF(bool, hyperlink_auditing_enabled, true)
WEB_PREF(bool, allow_universal_access_from_file_urls, false)
WEB_PREF(bool, allow_file_access_from_file_urls, false)
WEB_PREF(bool, allow_displaying_insecure_content, true)
WEB_PREF(bool, allow_running_insecure_content, false)
WEB_PREF(bool, password_echo_enabled, false)
WEB_PREF(bool, loads_images_automatically, true)
WEB_PREF(bool, text_areas_are_resizable, true)
WEB_PREF(bool, local_storage_enabled, false)
WEB_PREF(bool, application_cache_enabled, false)
WEB_PREF(bool, tabs_to_links, true)
#undef WEB_PREF_DEFAULT
