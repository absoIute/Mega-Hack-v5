# Mega-Hack-v5
'Shit tonna hacks.
# Running on Linux
```bash
cd_done=true
gmd=.local/share/Steam/steamapps/common/Geometry\ Dash
if [ -d ~/.var/app/com.valvesoftware.Steam/"$gmd" ]; then
    cd ~/.var/app/com.valvesoftware.Steam/"$gmd" # Flatpak Steam
elif [ -d ~/"$gmd" ]; then
    cd ~/"$gmd"
else
    echo "seems that you don't have GD installed through Steam"
    cd_done=false
fi
if $cd_done; then
    rm -rf Mega-Hack
    if command -v git &>/dev/null; then
    	git clone https://github.com/absoIute/Mega-Hack-v5.git Mega-Hack
        cp -rf Mega-Hack/bin/* ./
        rm -rf Mega-Hack
    else
        echo "please install git"
    fi
fi
```
Then go to GD *Properties...* in Steam and set launch options: `%command% & ~/.local/share/Steam/steamapps/common/Proton\ 3.7/proton waitforexitandrun ~/.local/share/Steam/steamapps/common/Geometry\ Dash/MegaHack.exe`
