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

#### Note aggiuntive

- L'AI attacca il nemico con meno HP tra quelli nel range di attacco, invece del più vicino. In questo modo l'AI si concentra sulle unità più deboli, cercando di eliminarle rapidamente per ridurre il numero di unità di HumanPlayer nella mappa.

## Descrizione dei file principali
Di seguito è fornita una descrizione dei file principali organizzata per categoria, con dettagli aggiuntivi sui file più rilevanti.

### Classi C++

| File           | Tipo  | Descrizione                           |
|----------------|-------|---------------------------------------|
| `Tile .h/.cpp` | Actor | Singola cella della griglia di gioco. |

### Blueprint

| Blueprint | Tipo            |
|-----------|-----------------|
| `BP_Tile` | Blueprint Actor |

### Materiali

| Materiale          | Descrizione                                                       |
|--------------------|-------------------------------------------------------------------|
| `M_Brawler_AI`     | Materiale per brawler AI                                          |
| `M_Brawler_Human`  | Materiale per brawler HumanPlayer                                 |
| `M_Floor`          | Materiale per sfondo livello                                      |
| `M_Sniper_AI`      | Materiale per sniper AI                                           |
| `M_Sniper_Human`   | Materiale per sniper HumanPlayer                                  |
| `M_Tile`           | Materiale per la singola cella della griglia                      |
| `M_Tower`          | Materiale per la torre                                            |
| `M_TowerContested` | Materiale per la torre contesa, con i colori di entrambi i player |

### Texture

| Texture                 | Descrizione                                     |
|-------------------------|-------------------------------------------------|
| `T_AiTowerCount`		  | Texture immagine per icona torre AI             |
| `T_Brawler_AI`		  | Brawler robot per AI                            |
| `T_Brawler_Human`		  | Brawler umano per HumanPlayer                   |
| `T_FloorLevel`		  | Texture immagine dello spazio usata come sfondo |
| `T_HowToPlayBackground` | Texture immagine per schermata how to play      |
| `T_HumanTowerCount`	  | Texture immagine per icona torre Human          |
| `T_Sniper_AI`			  | Sniper robot per AI                             |
| `T_Sniper_Human`        | Sniper umano per HumanPlayer                    |
| `T_VictoryBackground`   | Texture immagine per schermata vittoria         |

## Risorse utilizzate
- [Documentazione ufficiale Unreal Engine 5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-6-documentation?application_version=5.6)
- [Variables, Timers, and Events - UE 5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-guide-to-variables-timers-and-events-in-unreal-engine-cpp?application_version=5.6)
- [Making maps with noise functions from Red Blob Games](https://www.redblobgames.com/maps/terrain-from-noise/)
- [Top-down Shooter PNG pack from Kenney.nl](https://kenney.nl/assets/top-down-shooter)
- [Introduction to the A-star Algorithm from Red Blob Games](https://www.redblobgames.com/pathfinding/a-star/introduction.html)
- [A-star search algorithm Wikipedia](https://en.wikipedia.org/wiki/A*_search_algorithm#Pseudocode)
- [Hexagonal Grids - sezione Distances from Red Blob Games](https://www.redblobgames.com/grids/hexagons/#distances)
- [Chebyshev distance Wikipedia](https://en.wikipedia.org/wiki/Chebyshev_distance)
- [FString Chr - UE 5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Containers/FString/Chr?application_version=5.3)
- [Greedy Best first search algorithm from GeeksforGeeks](https://www.geeksforgeeks.org/dsa/greedy-best-first-search-algorithm/)
- [Greedy Best-First Search from CodeAcademy.com](https://www.codecademy.com/resources/docs/ai/search-algorithms/greedy-best-first-search)