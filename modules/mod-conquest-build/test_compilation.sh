#!/bin/bash

# Script de test de compilation du module GvG Build
# Usage: ./test_compilation.sh [path_to_azerothcore_build]

echo "========================================="
echo "Test de Compilation - GvG Build Module"
echo "========================================="

# Chemin du build d'AzerothCore
BUILD_DIR="${1:-../../build}"

if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Erreur: Dossier build non trouvé: $BUILD_DIR"
    echo "Usage: ./test_compilation.sh [path_to_azerothcore_build]"
    exit 1
fi

echo "📁 Dossier build: $BUILD_DIR"
echo ""

# Aller dans le dossier build
cd "$BUILD_DIR" || exit 1

echo "🔨 Lancement de la compilation..."
echo ""

# Compiler uniquement le module
make -j $(nproc) modules 2>&1 | tee /tmp/gvg_build_compile.log

# Vérifier le résultat
if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "✅ COMPILATION RÉUSSIE !"
    echo "========================================="
    echo ""
    echo "Prochaines étapes:"
    echo "1. make install"
    echo "2. Exécuter les scripts SQL"
    echo "3. Copier le fichier de configuration"
    echo "4. Redémarrer le serveur"
    echo ""
else
    echo ""
    echo "========================================="
    echo "❌ ERREUR DE COMPILATION"
    echo "========================================="
    echo ""
    echo "Log complet: /tmp/gvg_build_compile.log"
    echo ""
    echo "Erreurs trouvées:"
    grep -i "error:" /tmp/gvg_build_compile.log | tail -n 10
    echo ""
    exit 1
fi

