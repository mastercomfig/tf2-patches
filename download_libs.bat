set libs="lib/public/dme_controls.lib"
set server="https://libs.mastercomfig.com/"

(for %%l in (%libs%) do (
   curl %server%%%l -o %%l
))
PAUSE
