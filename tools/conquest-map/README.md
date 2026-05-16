# Conquest Map

Visualisation 2D des bannières de conquête (`gameobject.id = 400010`) et des
waypoints (`gameobject.id = 400100`) sur les cartes d'Azeroth, avec affichage
du contrôle de faction (Alliance / Horde / Neutre) lu depuis la table
`acore_characters.conquest_zone_control`.

Lecture seule. Aucune écriture sur la DB.

## Architecture

- `server/` — Node + Express + mysql2 + TypeScript. Lit `acore_world` et
  `acore_characters`, expose `/api/banners`, `/api/waypoints`, `/api/all`.
- `client/` — Vite + React + TypeScript. Affiche une carte par continent (EK,
  Kalimdor, Outland, Northrend), markers colorés par faction, lignes pour les
  arêtes waypoint (proximité < 500y, même map — règle copiée de
  `ConquestWaypointMgr`).

## Lancement

### 1. Backend

```bash
cd tools/conquest-map/server
cp .env.example .env   # ajuste DB_PASSWORD si nécessaire
npm install
npm run dev            # écoute sur :4317
```

Endpoints :
- `GET /api/health`
- `GET /api/banners`
- `GET /api/waypoints`
- `GET /api/all`

### 2. Frontend

```bash
cd tools/conquest-map/client
npm install
npm run dev            # http://localhost:5317, proxy /api → :4317
```

### 3. Images de carte

Déposer les 4 images JPG dans `client/public/maps/` :

- `0.jpg` — Eastern Kingdoms
- `1.jpg` — Kalimdor
- `530.jpg` — Outland
- `571.jpg` — Northrend

Sans images, un fond quadrillé est affiché (la projection reste correcte).

## Projection

Les bornes monde→pixel par continent sont définies dans
`client/src/projection.ts` (champ `bounds`). Si une bannière apparaît au
mauvais endroit sur ton image :

1. Repère un point connu (ex. Crossroads à `(-435, -2660)` sur Kalimdor).
2. Note où il devrait tomber en pixels sur ton image.
3. Ajuste `xMin/xMax/yMin/yMax` du continent concerné.

Convention WoW :
- `+X` = nord (haut de carte)
- `+Y` = ouest (gauche de carte)

## Limites connues

- Le graph waypoint est recalculé côté client (même règle que le serveur :
  arête si distance ≤ 500y et même map). Si tu changes la règle côté C++,
  pense à la refléter dans `client/src/graph.ts`.
- Pas d'auth ; à ne pas exposer publiquement tel quel.
