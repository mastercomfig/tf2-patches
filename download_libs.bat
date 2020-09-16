set libs="lib/public/dme_controls.lib" "lib/common/win32/2015/debug/cryptlib.lib" "lib/common/win32/2015/release/cryptlib.lib" "lib/common/x64/2015/debug/cryptlib.lib" "lib/common/x64/2015/release/cryptlib.lib"
set server="https://libs.mastercomfig.com/"

(for %%l in (%libs%) do (
   curl %server%%%l -o %%l
))
PAUSE
