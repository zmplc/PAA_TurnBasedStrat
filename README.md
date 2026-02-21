# Strategico a Turni 3D
### Progetto di Esame - Progettazione e Analisi Algoritmi

Questo progetto consiste nell'implementazione in Unreal Engine 5.6 di un gioco strategico a turni 3D, il progetto è sviluppato in C++ con integrazione di Blueprint.

### Requisiti implementati

* [ ] Il progetto compila correttamente, il codice è ben commentato e ben strutturato (polimorfismo ed ereditarietà).
* [ ] Griglia di gioco iniziale graficamente corretta e interamente visibile nello schermo.
* [ ] Meccanismo di posizionamento Unità di Gioco e Torri come da specifiche
* [ ] AI che utilizza algoritmo A* (movimento e attacco).
* [ ] Il gioco funziona a turni e termina quando un giocatore vince
* [ ] Interfaccia grafica rappresentante lo stato corrente del gioco (turno corrente, vita rimanente di ciascuna unità, Torri conquistate).
* [ ] Suggerimenti del range di movimento possibile per ciascuna unità cliccando sulla stessa, colorando opportunamente tutte le celle nel range.
* [ ] Implementazione del meccanismo del danno da contrattacco (guarda "Osservazioni e ulteriori specifiche").
* [ ] Lista dello storico delle mosse eseguite.
* [ ] AI che utilizza algoritmi euristici ottimizzati di movimento (diverso da A*).

## Descrizione dei file principali
Di seguito è fornita una descrizione dei file principali organizzata per categoria, con dettagli aggiuntivi sui file più rilevanti.

### Classi C++

| File           | Tipo  | Descrizione                           | Utilizzo |
|----------------|-------|---------------------------------------|----------|
| `Tile .h/.cpp` | Actor | Singola cella della griglia di gioco. |          |
|                |       |                                       |          |
|                |       |                                       |          |

### Blueprint

| Blueprint | Tipo            |
|-----------|-----------------|
| `BP_Tile` | Blueprint Actor |
|           |                 |
|           |                 |

### Materiali

| Materiale | Descrizione                                   |
|-----------|-----------------------------------------------|
| `M_Tile`  | Materiale per la singola cella della griglia. |
|           |                                               |
|           |                                               |

### Texture

| Texture | Descrizione |
|---------|-------------|
|         |             |
|         |             |
|         |             |

## Risorse utilizzate
- [Documentazione ufficiale Unreal Engine 5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-6-documentation?application_version=5.6)
- [Making maps with noise functions from Red Blob Games](https://www.redblobgames.com/maps/terrain-from-noise/)
- [Top-down Shooter PNG pack from Kenney.nl](https://kenney.nl/assets/top-down-shooter)
- [Introduction to the A-star Algorithm from Red Blob Games](https://www.redblobgames.com/pathfinding/a-star/introduction.html)
- [A-star search algorithm Wikipedia](https://en.wikipedia.org/wiki/A*_search_algorithm#Pseudocode)
