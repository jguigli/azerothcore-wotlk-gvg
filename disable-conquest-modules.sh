#!/usr/bin/env bash
#
# Désactive les modules GvG en renommant à la fois :
#   - le dossier src/ → src.disabled/   (bloque ConfigureModules.cmake / nouveau API)
#   - le CMakeLists.txt → .disabled     (bloque AC_ADD_SCRIPT_LOADER / ancien API)
#
# Les deux mécanismes coexistent dans AzerothCore et le fork Playerbot,
# il faut désactiver les deux pour qu'un module ne soit plus inclus dans le build.
#
# Garde mod-conquest-zone-control et mod-conquest-gear-manager actifs.
#
# Usage:
#   ./disable-gvg-modules.sh           # désactive
#   ./disable-gvg-modules.sh --restore # réactive
#

set -euo pipefail

MODULES_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/modules"

# Modules à désactiver (src/ → src.disabled/)
TO_DISABLE=(
    "mod-conquest-build"
    "mod-conquest-core"
    "mod-conquest-guard"
    "mod-conquest-limit-group"
    "mod-conquest-lootdrop"
    "mod-conquest-mounts"
    "mod-conquest-npc-vendor"
)

# Modules conservés (pour info)
KEPT=(
    "mod-conquest-zone-control"
    "mod-conquest-gear-manager"
    "mod-playerbots"
    "mod-transmog"
)

action="${1:-disable}"

if [[ "$action" == "--restore" ]]; then
    echo "→ Réactivation des modules Conquest"
    for mod in "${TO_DISABLE[@]}"; do
        # Restaure src/
        src_dis="$MODULES_DIR/$mod/src.disabled"
        src="$MODULES_DIR/$mod/src"
        if [[ -d "$src_dis" ]]; then
            mv "$src_dis" "$src"
        fi

        # Restaure CMakeLists.txt
        cml_dis="$MODULES_DIR/$mod/CMakeLists.txt.disabled"
        cml="$MODULES_DIR/$mod/CMakeLists.txt"
        if [[ -f "$cml_dis" ]]; then
            mv "$cml_dis" "$cml"
        fi

        if [[ -d "$src" && -f "$cml" ]]; then
            echo "  ✓ $mod réactivé (src/ + CMakeLists.txt)"
        else
            echo "  ⚠ $mod : état incohérent — vérifier manuellement"
        fi
    done
    echo "Done."
    exit 0
fi

echo "→ Désactivation des modules Conquest (sauf zone-control et gear-manager)"
for mod in "${TO_DISABLE[@]}"; do
    actions=()

    # Désactive src/
    src="$MODULES_DIR/$mod/src"
    src_dis="$MODULES_DIR/$mod/src.disabled"
    if [[ -d "$src" ]]; then
        mv "$src" "$src_dis"
        actions+=("src/")
    fi

    # Désactive CMakeLists.txt
    cml="$MODULES_DIR/$mod/CMakeLists.txt"
    cml_dis="$MODULES_DIR/$mod/CMakeLists.txt.disabled"
    if [[ -f "$cml" ]]; then
        mv "$cml" "$cml_dis"
        actions+=("CMakeLists.txt")
    fi

    if [[ ${#actions[@]} -gt 0 ]]; then
        echo "  ✓ $mod désactivé (${actions[*]})"
    elif [[ -d "$src_dis" || -f "$cml_dis" ]]; then
        echo "  · $mod déjà désactivé"
    else
        echo "  ⚠ $mod : aucun fichier à désactiver"
    fi
done

echo ""
echo "Modules conservés actifs :"
for mod in "${KEPT[@]}"; do
    if [[ -d "$MODULES_DIR/$mod/src" ]]; then
        echo "  ✓ $mod"
    elif [[ -d "$MODULES_DIR/$mod" ]]; then
        echo "  ⚠ $mod existe mais pas de src/"
    else
        echo "  ⚠ $mod absent du dossier modules/"
    fi
done

echo ""
echo "Pour réactiver : $0 --restore"
echo "Pense à lancer le build (make all) après."
