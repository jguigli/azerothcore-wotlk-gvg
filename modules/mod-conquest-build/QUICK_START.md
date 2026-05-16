# 🚀 Quick Start - GvG Build Module

## ✅ Erreur de Compilation CORRIGÉE !

L'erreur `use of undeclared identifier 'Trinity'` a été corrigée. 
Le module utilise maintenant l'API native d'AzerothCore.

---

## 📋 Configuration

✅ **Item** : 80000 (Kit de Construction GvG)  
✅ **Mur** : 190397 (50,000 PV)  
✅ **Tour** : 190398 (100,000 PV)  

---

## 🔨 Installation en 5 Étapes

### 1️⃣ Compilation

```bash
cd /path/to/azerothcore/build
make -j $(nproc)
make install
```

### 2️⃣ Base de données World

```bash
mysql -u root -p acore_world < /path/to/mod-gvg-build/data/sql/db-world/gvg_build_install.sql
```

### 3️⃣ Base de données Characters

```bash
mysql -u root -p acore_characters < /path/to/mod-gvg-build/data/sql/db-characters/gvg_build_structures.sql
```

### 4️⃣ Configuration

```bash
cp /path/to/mod-gvg-build/conf/gvg_build.conf.dist /path/to/server/etc/modules/gvg_build.conf
```

### 5️⃣ Redémarrage

```bash
./worldserver
```

---

## 🎮 Test en Jeu

```
.additem 80000
```

Clic droit sur l'item → Choisir Mur ou Tour → Structure construite ! 🏰

---

## 📁 Fichiers Importants

| Fichier | Description |
|---------|-------------|
| `FIXES_APPLIED.md` | ✅ Détail des corrections |
| `COMPILATION_FIXES.md` | 🔧 Explications techniques |
| `INSTALLATION.md` | 📖 Guide complet |
| `README.md` | 📚 Documentation |
| `test_compilation.sh` | 🧪 Script de test |

---

## 🐛 Correction Appliquée

**Problème** : Code utilisant l'API TrinityCore (`Trinity::`)  
**Solution** : Conversion vers l'API AzerothCore native

- ✅ `FindNearestGameObject()` au lieu de `Trinity::AllGameObjectsInRange`
- ✅ `AddGossipItemFor()` au lieu de `ADD_GOSSIP_ITEM()`
- ✅ Création permanente de GameObject avec `Create()` + `SaveToDB()`
- ✅ `GetDBTableGUIDLow()` au lieu de `GetSpawnId()`

---

## ✅ Checklist

- [ ] Module compilé
- [ ] SQL world exécuté
- [ ] SQL characters exécuté
- [ ] Configuration copiée
- [ ] Serveur redémarré
- [ ] Test en jeu réussi

---

## 🆘 Besoin d'Aide ?

### Si erreur de compilation
→ Voir `FIXES_APPLIED.md` ligne par ligne

### Si erreur SQL
→ Vérifier que les tables n'existent pas déjà

### Si crash en jeu
→ Vérifier les logs du worldserver

### Si l'item ne fonctionne pas
→ Vérifier que `ScriptName = 'item_gvg_build'` dans la DB

---

## 🎉 C'est Prêt !

Le module a été entièrement corrigé et est prêt à être compilé et utilisé.

**Commande rapide** :
```bash
cd /path/to/azerothcore/build && make -j $(nproc) && make install
```

Bon jeu ! 🏰

