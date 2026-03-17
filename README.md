# Strategico a Turni 3D
### Progetto di Esame - Progettazione e Analisi Algoritmi

Questo progetto consiste nell'implementazione in Unreal Engine 5.6 di un gioco strategico a turni 3D, il progetto è sviluppato in C++ con integrazione di Blueprint.

### Requisiti implementati

* [ ] Il progetto compila correttamente, il codice è ben commentato e ben strutturato (polimorfismo ed ereditarietà).
* [x] Griglia di gioco iniziale graficamente corretta e interamente visibile nello schermo.
* [x] Meccanismo di posizionamento Unità di Gioco e Torri come da specifiche
* [x] AI che utilizza algoritmo A* (movimento e attacco).
* [x] Il gioco funziona a turni e termina quando un giocatore vince
* [x] Interfaccia grafica rappresentante lo stato corrente del gioco (turno corrente, vita rimanente di ciascuna unità, Torri conquistate).
* [x] Suggerimenti del range di movimento possibile per ciascuna unità cliccando sulla stessa, colorando opportunamente tutte le celle nel range.
* [x] Implementazione del meccanismo del danno da contrattacco (guarda "Osservazioni e ulteriori specifiche").
* [x] Lista dello storico delle mosse eseguite.
* [x] AI che utilizza algoritmi euristici ottimizzati di movimento (diverso da A*).

### Note aggiuntive

- RandomPlayer è l'AI che utilizza l'algoritmo A* per muoversi e attaccare.
- HeuristicPlayer è l'AI che utilizza algoritmi euristici ottimizzati di movimento.
- L'AI attacca il nemico con meno HP tra quelli nel range di attacco, invece del più vicino. In questo modo l'AI si concentra sulle unità più deboli, cercando di eliminarle rapidamente per ridurre il numero di unità di HumanPlayer nella mappa.
- Le barre della vità delle unità sono sempre visibili a schermo. In base ai punti vita delle unità le barre cambiano colore e dimensione: verde, giallo e rosso.
- Nella schermata di configurazione della mappa, l'utente può scegliere i parametri dei 5 livelli della mappa (acqua, terreno, collina e montagne).

### HeuristicPlayer

**HeuristicPlayer** è la classe che implementa un'AI basata su algoritmi euristici ottimizzati di movimento. Invece di utilizzare l'algoritmo A* (usato invece in RandomPlayer), HeuristicPlayer valuta le mosse possibili in base a funzioni euristiche che tengono conto di diversi fattori:
- distanza dalle torri
- stato delle torri (neutrali, conquistate o contese)
- distanza dalle unità nemiche
- HP delle unità nemiche
- range di attacco delle unità nemiche (siccome lo sniper attacca a distanza, l'AI cerca di attaccare lo sniper prima del brawler)

Queste funzioni assegnano un punteggio a ciascuna mossa possibile e l'AI sceglie la mossa con il punteggio più alto, cercando così di massimizzare le proprie possibilità di vittoria.

### Movimento interpolato (Lerp)

Il movimento delle unità è gestito tramite **Interpolazione Lineare** `Lerp` all'interno del metodo `Tick()` della classe `Unit`. per garantire un movimento fluido, evitando così che le unità si teletrasportino istantaneamente da una cella all'altra.

#### Logica di funzionamento

Il sistema calcola la posizione dell'unità in ogni frame, interpolando tra la posizione attuale e la posizione di destinazione, utilizzando i seguenti parametri:
- **Alpha**: valore normalizzato tra `0.0f` a `1.0f` che rappresenta la progressione del movimento. Viene calcolato come il rapporto tra il tempo trascorso (`MovementElapsedTime`) dall'inizio del movimento e la durata totale del movimento tra le due celle (`MovementDuration`).
- **MovementStartLocation**: posizione iniziale dell'unità all'inizio del movimento.
- **MovementTargetLocation**: posizione finale che l'unità deve raggiungere.
- **`FMath::Lerp`**: funzione di Unreal Engine che calcola la posizione interpolata in base a `Alpha`, `MovementStartLocation` e `MovementTargetLocation`. Calcola la posizione intermedia dell'unità in ogni frame, in base al valore di Alpha, creano così un movimento fluido ed evitando il teletrasporto istantaneo.
- **Pathfinding**: l'unità deve seguire un percorso di tile, quindi il sistema gestisce una coda di posizioni di destinazione (le celle del percorso per arrivare alla target tile). Una volta raggiunto `Alpha >= 1.0f` per la cella corrente, viene impostata la destinazione precedente come nuova partenza e l'indice dell'array `[CurrentPathIndex]` viene incrementato per passare alla cella successiva del percorso, finché l'intero array `MovementPath` non viene esaurito.

**Osservazione**: dopo il controllo su `Alpha >= 1.0f` viene effettuata l'assegnazione esplicita dell'unità alla posizione target con `SetActorLocation(MovementTargetLocation);`, questo viene fatto per garantire che l'unità raggiunga esattamente la posizione target ed evitare bug nel posizionamento, siccome nel calcolo di `NewLocation` tramite `Lerp` potrebbero verificarsi errori di approssimazione nel calcolo dell'interpolazione.

## Descrizione dei file principali
Di seguito è fornita una descrizione dei file principali organizzata per categoria, con dettagli aggiuntivi sui file più rilevanti.

### Classi C++

| File							| Tipo						   |
|-------------------------------|------------------------------|
| `ConfigData.h/.cpp`			| Data Asset				   |
| `GameField.h/.cpp`			| Actor						   |
| `HeuristicPlayer.h/.cpp`		| Pawn						   |
| `HumanPlayer.h/.cpp`			| Pawn						   |
| `PlayerInterface.h`			| Interface					   |
| `RandomPlayer.h/.cpp`			| Pawn						   |
| `TBS_GameInstance.h/.cpp`     | Game Instance				   |
| `TBS_GameMode.h/.cpp`         | Game Mode					   |
| `TBS_PlayerController.h/.cpp` | Player Controller			   |
| `Tile.h/.cpp`					| Actor						   |
| `Tower.h/.cpp`				| Actor						   |
| `Unit.h/.cpp`					| Pawn (Base Class)			   |
| `UnitBrawler.h/.cpp`			| Pawn (inherits from `AUnit`) |
| `UnitSniper.h/.cpp`			| Pawn (inherits from `AUnit`) |

### Blueprint

| Blueprint            | Tipo              |
|----------------------|-------------------|
| `BP_Brawler_AI`      | Pawn              |
| `BP_Brawler_Human`   | Pawn              |
| `BP_GameField`       | Actor             |
| `BP_GameInstance`    | Game Instance     |
| `BP_GameMode`        | Game Mode         |
| `BP_HeuristicPlayer` | Pawn              |
| `BP_HumanPlayer`     | Pawn              |
| `BP_MenuGameMode`    | Game Mode         |
| `BP_PlayerController`| Player Controller |
| `BP_RandomPlayer`    | Pawn              |
| `BP_Sniper_AI`       | Pawn              |
| `BP_Sniper_Human`    | Pawn              |
| `BP_Tile`            | Actor             |
| `BP_Tower`           | Actor             |
| `DA_ConfigData`      | Data Asset        |
| `HowToPlay_TBS`      | Widget Blueprint  |
| `HUD_TBS`            | Widget Blueprint  |
| `MainMenu_TBS`       | Widget Blueprint  |
| `MapConfig_TBS`      | Widget Blueprint  |

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
| `T_Bullet`		      | Texture immagine per proiettile                 |
| `T_FloorLevel`		  | Texture immagine dello spazio usata come sfondo |
| `T_HumanTowerCount`	  | Texture immagine per icona torre Human          |
| `T_Knife`		          | Texture immagine per arma corpo a corpo         |
| `T_MainBackground`      | Texture immagine per le schermate widget HUD    |
| `T_Sniper_AI`			  | Sniper robot per AI                             |
| `T_Sniper_Human`        | Sniper umano per HumanPlayer                    |
| `T_VictoryBackground`   | Texture immagine per schermata vittoria         |

## Risorse utilizzate

- [Documentazione ufficiale Unreal Engine 5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-6-documentation?application_version=5.6)
- [Variables, Timers, and Events - Documentazione UE 5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-guide-to-variables-timers-and-events-in-unreal-engine-cpp?application_version=5.6)
- [Making maps with noise functions from Red Blob Games](https://www.redblobgames.com/maps/terrain-from-noise/)
- [Top-down Shooter PNG pack from Kenney.nl](https://kenney.nl/assets/top-down-shooter)
- [Introduction to the A-star Algorithm from Red Blob Games](https://www.redblobgames.com/pathfinding/a-star/introduction.html)
- [A-star search algorithm Wikipedia](https://en.wikipedia.org/wiki/A*_search_algorithm#Pseudocode)
- [Hexagonal Grids - sezione Distances from Red Blob Games](https://www.redblobgames.com/grids/hexagons/#distances)
- [Chebyshev distance Wikipedia](https://en.wikipedia.org/wiki/Chebyshev_distance)
- [FString Chr - Documentazione UE](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Containers/FString/Chr?application_version=5.3)
- [Greedy Best first search algorithm from GeeksforGeeks](https://www.geeksforgeeks.org/dsa/greedy-best-first-search-algorithm/)
- [Greedy Best-First Search from CodeAcademy.com](https://www.codecademy.com/resources/docs/ai/search-algorithms/greedy-best-first-search)
- [Lerp - Documentazione UE](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Math/FMath/Lerp?application_version=5.3)

## Configurazione iniziale

Seguire i seguenti passaggi per configurare l'ambiente di sviluppo e compilare il progetto:

1. **Generazione della soluzione Visual Studio**: il progetto non include il file `.sln`. Per crearlo è sufficiente individuare il file `.uproject` nella cartella principale del progetto, cliccare con il tasto destro e selezionare "Generate Visual Studio project files".
2. **Starter Content**: il progetto utilizza asset provenienti dallo "Starter Content" di Unreal Engine. Per importare questi asset, aprire il progetto in Unreal Engine, andare nella sezione "Content Browser", cliccare sul pulsante "Add" e selezionare "Add Feature or Content Pack". Nella finestra che si apre, selezionare la tab "Content" e scegliere "Starter Content", cliccare poi su "Add to Project".
3. **Prima compilazione**: dopo aver generato la soluzione Visual Studio, aprire il file `.sln`, selezionare la configurazione "Development Editor" e compilare il progetto.