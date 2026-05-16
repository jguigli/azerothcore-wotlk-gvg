# 🏰 Mod PvP GvG — Concept Global

## 🎯 Objectif

Créer un serveur PvP centré sur :

- les guildes
- la domination territoriale
- le risque / récompense
- des combats dynamiques et constants

---

## 👥 Guildes & Bases

### 🏗️ Création de base

- Chaque guilde possède une base unique
- Elle est placée par le chef de guilde
- Elle sert de :
  - point de respawn
  - hub principal
  - centre de progression

### 💀 PNJ central de base (cœur)

- La base contient un PNJ principal
- Si ce PNJ est tué :
  - ❌ les membres de la guilde ne peuvent plus respawn dans la base
  - ✅ ils respawn au spawn par défaut

> 👉 Crée une vraie mécanique de siège / défense.

### 🧱 Évolution de la base

Via un PNJ gestionnaire de base :

- Amélioration progressive de la base
- La base :
  - s'agrandit automatiquement
  - débloque du contenu
- Déblocages possibles :
  - PNJ marchands
  - PNJ d'améliorations
  - Véhicules
  - Défenses
  - Accès à du meilleur équipement

---

## ⚔️ Système de Stuff

### 🟢 Stuff de base

- Accessible facilement
- Basé sur du **S6**
- Permet à tous les joueurs d'être compétitifs

### 🔵 Stuff avancé

- Basé sur du **S8**
- Doit être acheté avec des points
- Plus puissant
- **Risque :**
  - Peut être perdu à la mort
  - Peut être loot par d'autres joueurs

> 👉 Ajoute de la tension sans bloquer le gameplay.

---

## 🔁 Boucle de gameplay (résumé)

1. Les joueurs rejoignent une guilde
2. Développent leur base
3. Participent aux combats / objectifs
4. Gagnent des points
5. Investissent dans :
   - leur base
   - leur équipement
6. Attaquent / défendent des bases ennemies

---

## 🧠 Philosophie du système

- ✔ **Accessible** — stuff de base toujours dispo
- ✔ **Risqué** — stuff avancé perdu à la mort
- ✔ **Dynamique** — bases attaquables
- ✔ **Progressif** — évolution de guilde

---

# 🗺️ Système de capture de zones

## 🎯 Objectif

Créer des points de conflit permanents sur la map.

## ⚙️ Fonctionnement

### 📍 Zones capturables

- Plusieurs zones réparties dans le monde
- Chaque zone contient une bannière

### 🏳️ Capture

- Un joueur interagit avec la bannière
- Un timer démarre (ex : 10–20 secondes)
- Si interrompu → reset

> 👉 Une fois capturée : la zone appartient à la guilde.

### 🔄 Génération de ressources

> 👉 **IMPORTANT** : les zones ne donnent **PAS** directement des points.

Elles produisent :

- des ressources (points bruts) dans un stock local
- production lente et continue
- limite de stockage (pour forcer l'action)

### 🚚 Collecte

- Les joueurs doivent venir récupérer les ressources
- Elles sont placées dans leur inventaire

> 👉 À partir de là : ils deviennent des cibles prioritaires.

### ⚠️ Risque

Si le joueur meurt :

- il drop les ressources
- les ennemis peuvent les récupérer

### 🎯 Résultat

- crée des fights naturels
- évite le gameplay AFK
- force les déplacements

---

# 🏴 Système de vol de flag (bases)

## 🎯 Objectif

Créer des moments de guerre majeurs entre guildes.

## 🏰 Dans chaque base

- Un drapeau de guilde est présent
- Protégé par :
  - joueurs
  - PNJ
  - structures

## 🥷 Vol du drapeau

- Un joueur ennemi peut interagir avec le flag
- Canalisation (ex : 10 sec)
- Si réussi :
  - il prend le flag
  - devient porteur

## 🧠 Effets immédiats

> 👉 Quand le flag est volé :

- 📢 annonce serveur
- 📍 position approximative du porteur (optionnel mais conseillé)

## 🐢 Contraintes du porteur

Pour équilibrer :

- vitesse réduite
- pas de monture
- visible / marqué

> 👉 Devient une cible.

## 🎯 Objectif

Ramener le flag dans sa base.

## 🏆 Récompense

Si réussi :

- **Pour la guilde attaquante :**
  - gain massif de points
- **Pour la guilde attaquée :**
  - perte de points **OU**
  - debuff temporaire, exemples :
    - respawn plus lent
    - production réduite
    - PNJ affaiblis

## 💀 Si le porteur meurt

Le flag tombe au sol et peut être :

- récupéré par un allié
- récupéré par les défenseurs
- reset après un temps

---

# 🔗 Lien entre les deux systèmes

> 👉 **Les zones** servent à :
> - générer des ressources
> - renforcer ta guilde

> 👉 **Le flag** sert à :
> - mettre un gros coup à l'ennemi

## 🔁 Boucle complète

```
Capture zone → génère ressources → transport →
améliore base → attaque ennemie → vole flag →
guerre → défense → revanche
```

## ⚖️ Équilibrage simple (important)

> 👉 À garder en tête :
> - **Zones** = revenus réguliers
> - **Flag** = impact ponctuel fort

## 🧠 Résumé

- ✔ **Zones** → activité constante
- ✔ **Transport** → tension
- ✔ **Flag** → moments épiques
- ✔ **Bases** → enjeux réels
