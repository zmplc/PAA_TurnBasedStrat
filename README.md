# Strategico a Turni 3D
### Progetto di Esame - Progettazione e Analisi Algoritmi

Questo progetto consiste nell'implementazione in Unreal Engine 5.6 di un gioco strategico a turni 3D, il progetto è sviluppato in C++ con integrazione di Blueprint.

## Requisiti implementati

* [x] Il progetto compila correttamente, il codice è ben commentato e ben strutturato (polimorfismo ed ereditarietà).
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

## Osservazioni e ulteriori specifiche

### HeuristicPlayer

**HeuristicPlayer** è la classe che implementa un'AI basata su algoritmi euristici ottimizzati di movimento. Invece di utilizzare l'algoritmo A* (usato invece in RandomPlayer), HeuristicPlayer valuta le mosse possibili in base a funzioni euristiche che tengono conto di diversi fattori:
- distanza dalle torri
- stato delle torri (neutrali, conquistate o contese)
- distanza dalle unità nemiche
- HP delle unità nemiche
- range di attacco delle unità nemiche (siccome lo sniper attacca a distanza, l'AI cerca di attaccare lo sniper prima del brawler)

Queste funzioni assegnano un punteggio a ciascuna mossa possibile e l'AI sceglie la mossa con il punteggio più alto, cercando così di massimizzare le proprie possibilità di vittoria.

### Movimento interpolato (Lerp)

Il movimento delle unità è gestito tramite **Interpolazione Lineare** `Lerp` all'interno del metodo `Tick()` della classe `Unit`, per garantire un movimento fluido, evitando così che le unità si teletrasportino istantaneamente da una cella all'altra.

#### Logica di funzionamento

Il sistema calcola la posizione dell'unità in ogni frame, interpolando tra la posizione attuale e la posizione di destinazione, utilizzando i seguenti parametri:
- **Alpha**: valore normalizzato tra `0.0f` a `1.0f` che rappresenta la progressione del movimento. Viene calcolato come il rapporto tra il tempo trascorso (`MovementElapsedTime`) dall'inizio del movimento e la durata totale del movimento tra le due celle (`MovementDuration`).
- **MovementStartLocation**: posizione iniziale dell'unità all'inizio del movimento.
- **MovementTargetLocation**: posizione finale che l'unità deve raggiungere.
- **`FMath::Lerp`**: funzione di Unreal Engine che calcola la posizione interpolata in base a `Alpha`, `MovementStartLocation` e `MovementTargetLocation`. Calcola la posizione intermedia dell'unità in ogni frame, in base al valore di Alpha, creano così un movimento fluido ed evitando il teletrasporto istantaneo.
- **Pathfinding**: l'unità deve seguire un percorso di tile, quindi il sistema gestisce una coda di posizioni di destinazione (le celle del percorso per arrivare alla target tile). Una volta raggiunto `Alpha >= 1.0f` per la cella corrente, viene impostata la destinazione precedente come nuova partenza e l'indice dell'array `[CurrentPathIndex]` viene incrementato per passare alla cella successiva del percorso, finché l'intero array `MovementPath` non viene esaurito.

**Osservazione**: dopo il controllo su `Alpha >= 1.0f` viene effettuata l'assegnazione esplicita dell'unità alla posizione target con `SetActorLocation(MovementTargetLocation);`, questo viene fatto per garantire che l'unità raggiunga esattamente la posizione target ed evitare bug nel posizionamento, siccome nel calcolo di `NewLocation` tramite `Lerp` potrebbero verificarsi errori di approssimazione nel calcolo dell'interpolazione.

### Attack Indicator

La classe `AttackIndicator` è un **Actor** che fornisce un feedback visivo al giocatore, evidenziando quali unità nemiche sono attualmente nel range di attacco dell'unità selezionata. L'actor è composto da una `USceneComponent` e una `UStaticMeshComponent` che visualizza l'icona. Questo aiuta il giocatore a prendere decisioni strategiche con più informazioni possibili. 

#### Logica di funzionamento

Attraverso la funzione `SetTargetUnit(AUnit* TargetUnit)`, l'icona viene vincolata ad una specifica unità nemica. Viene applicato un offset verticale costante `300.0f`, in modo che l'icona appaia sopra l'unità target. Tutte le collisioni sono disabilitate per quest attore, in modo che non interferisca con il gameplay e siccome è solo un feedback visivo.

L'aspetto importante è la gestione del ciclo di vita dell'Attack Indicator tramite la funzione `Tick(float DeltaTime)`. Ad ogni frame, viene eseguito un controllo sull'unità target:

1. **Esistenza dell'unità**: se l'unità target è stata distrutta (con `!IsValid(Target)`)
2. **Stato dell'unità**: se l'unità target è ancora attiva nel gioco (con `!TargetUnit->IsAlive()`)

Se l'unità target non è più valida o non è più viva, l'Attack Indicator chiama automaticamente `Destroy()`, rimuovendosi quindi dalla scena. Questo garantisce che l'indicatore non rimanga visibile quando l'unità target è stata eliminata, evitando di avere icone sopra unità non più presenti.

## Configurazione iniziale

Seguire i seguenti passaggi per configurare l'ambiente di sviluppo e compilare il progetto:

1. **Generazione della soluzione Visual Studio**: il progetto non include il file `.sln`. Per crearlo è sufficiente individuare il file `.uproject` nella cartella principale del progetto, cliccare con il tasto destro e selezionare "Generate Visual Studio project files".
2. **Starter Content**: il progetto utilizza asset provenienti dallo "Starter Content" di Unreal Engine. Per importare questi asset, aprire il progetto in Unreal Engine, andare nella sezione "Content Browser", cliccare sul pulsante "Add" e selezionare "Add Feature or Content Pack". Nella finestra che si apre, selezionare la tab "Content" e scegliere "Starter Content", cliccare poi su "Add to Project".
3. **Prima compilazione**: dopo aver generato la soluzione Visual Studio, aprire il file `.sln`, selezionare la configurazione "Development Editor" e compilare il progetto.

## Descrizione dei file principali
Di seguito è fornita una descrizione dei file principali organizzata per categoria, con dettagli aggiuntivi sui file più rilevanti.

### Classi C++

| File							| Tipo						   |
|-------------------------------|------------------------------|
| `AttackIndicator.h/.cpp`		| Actor						   |
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

### Audio

Per quanto riguarda l'audio, nel progetto ho utilizzato Sound Cue per gestire gli effetti sonori. In particolare per gli effetti sonori degli attacchi e delle morti delle unità, ho applicato un modulatore (impostando un pitch minimo di 0.9 e un pitch massimo di 1.1) in modo da evitare che i suoni risultino ripetitivi.

| File			       | Tipo	   | Descrizione													       | Percorso		   |
|----------------------|-----------|-----------------------------------------------------------------------|-------------------|
| `ClickButton_Cue`    | Sound Cue | Effetto sonoro per click dei bottoni								   | `Content/Audios/` |
| `Gun_Shot_Cue`       | Sound Cue | Effetto sonoro dello sparo per l'unità _Human Sniper_				   | `Content/Audios/` |
| `Human_Death_Cue`    | Sound Cue | Effetto sonoro riprodotto alla morte delle unità di _HumanPlayer_	   | `Content/Audios/` |
| `Human_Melee_Cue`    | Sound Cue | Effetto sonoro dell'attacco corpo a corpo per l'unità _Human Brawler_ | `Content/Audios/` |
| `Robot_Death_Cue`	   | Sound Cue | Effetto sonoro riprodotto alla morte delle unità dell'AI			   | `Content/Audios/` |
| `Robot_Gun_Shot_Cue` | Sound Cue | Effetto sonoro dello sparo per l'unità _AI Sniper_					   | `Content/Audios/` |
| `Robot_Melee_Cue`    | Sound Cue | Effetto sonoro dell'attacco corpo a corpo per l'unità _AI Brawler_    | `Content/Audios/` |

### Blueprint

| Blueprint            | Tipo              | Percorso			   |
|----------------------|-------------------|-----------------------|
| `BP_AttackIndicator` | Actor             | `Content/Blueprints/` |
| `BP_Brawler_AI`      | Pawn              | `Content/Blueprints/` |
| `BP_Brawler_Human`   | Pawn              | `Content/Blueprints/` |
| `BP_GameField`       | Actor             | `Content/Blueprints/` |
| `BP_GameInstance`    | Game Instance     | `Content/Blueprints/` |
| `BP_GameMode`        | Game Mode         | `Content/Blueprints/` |
| `BP_HeuristicPlayer` | Pawn              | `Content/Blueprints/` |
| `BP_HumanPlayer`     | Pawn              | `Content/Blueprints/` |
| `BP_MenuGameMode`    | Game Mode         | `Content/Blueprints/` |
| `BP_PlayerController`| Player Controller | `Content/Blueprints/` |
| `BP_RandomPlayer`    | Pawn              | `Content/Blueprints/` |
| `BP_Sniper_AI`       | Pawn              | `Content/Blueprints/` |
| `BP_Sniper_Human`    | Pawn              | `Content/Blueprints/` |
| `BP_Tile`            | Actor             | `Content/Blueprints/` |
| `BP_Tower`           | Actor             | `Content/Blueprints/` |
| `DA_ConfigData`      | Data Asset        | `Content/Blueprints/` |
| `HowToPlay_TBS`      | Widget Blueprint  | `Content/Blueprints/` |
| `HUD_TBS`            | Widget Blueprint  | `Content/Blueprints/` |
| `MainMenu_TBS`       | Widget Blueprint  | `Content/Blueprints/` |
| `MapConfig_TBS`      | Widget Blueprint  | `Content/Blueprints/` |

### Fonts

| Font			     | Descrizione									   | Percorso		  |
|--------------------|-------------------------------------------------|------------------|
| `Font_Main`		 | Font principale utilizzato per i testi dell'HUD | `Content/Fonts/` |
| `Font_Main_Narrow` | Variante del font principale a largheza ridotta | `Content/Fonts/` |

### Input

| Asset				 | Descrizione																| Percorso		   |
|--------------------|--------------------------------------------------------------------------|------------------|
| `IA_Click`		 | Azione associata al click sinistro del mouse								| `Content/Input/` |
| `IA_SelectBrawler` | Azione associata al tasto 2 per la selezione del brawler					| `Content/Input/` |
| `IA_SelectSniper`  | Azione associata al tasto 1 per la selezione dello sniper				| `Content/Input/` |
| `IMC_Context`		 | Input Mapping Context che mappa le azioni sopra indicate ai tasti fisici | `Content/Input/` |

### Livelli

| Livello			| Descrizione																															  | Percorso	      |
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------|-------------------|
| `Level_GameField` | Livello principale contenente la griglia di gioco, la logica di combattimento tra le unità e il sistema di conquista delle torri        | `Content/Levels/` |
| `MainMenu`		| Scena iniziale dedicata alla gestione dell'avvio del gioco, scelta dei parametri per la generazione della mappa e dell'AI da affrontare | `Content/Levels/` |

### Materiali

| Materiale          | Descrizione                                                       | Percorso				|
|--------------------|-------------------------------------------------------------------|----------------------|
| `M_AttackIcon`     | Materiale per icona target attacco                                | `Content/Materials/` |
| `M_Brawler_AI`     | Materiale per brawler AI                                          | `Content/Materials/` |
| `M_Brawler_Human`  | Materiale per brawler HumanPlayer                                 | `Content/Materials/` |
| `M_Floor`          | Materiale per sfondo livello                                      | `Content/Materials/` |
| `M_Sniper_AI`      | Materiale per sniper AI                                           | `Content/Materials/` |
| `M_Sniper_Human`   | Materiale per sniper HumanPlayer                                  | `Content/Materials/` |
| `M_Tile`           | Materiale per la singola cella della griglia                      | `Content/Materials/` |
| `M_Tower`          | Materiale per la torre                                            | `Content/Materials/` |
| `M_TowerContested` | Materiale per la torre contesa, con i colori di entrambi i player | `Content/Materials/` |

### Texture

| Texture                 | Descrizione											  | Percorso               |
|-------------------------|-------------------------------------------------------|------------------------|
| `T_AiTowerCount`		  | Texture immagine per icona torre AI					  | `Content/Textures/`    |
| `T_AttackIcon`		  | Texture per icona target attacco					  | `Content/Textures/`    |
| `T_Brawler_AI`		  | Brawler robot per AI								  | `Content/Textures/`    |
| `T_Brawler_Human`		  | Brawler umano per HumanPlayer						  | `Content/Textures/`    |
| `T_Bullet`		      | Texture immagine per proiettile						  | `Content/Textures/`    |
| `T_FloorLevel`		  | Texture immagine dello spazio usata come sfondo		  | `Content/Textures/`    |
| `T_HumanTowerCount`	  | Texture immagine per icona torre Human				  | `Content/Textures/`    |
| `T_Knife`		          | Texture immagine per arma corpo a corpo				  | `Content/Textures/`    |
| `T_MainBackground`      | Texture immagine per le schermate widget HUD		  | `Content/Textures/`    |
| `T_Sniper_AI`			  | Sniper robot per AI									  | `Content/Textures/`    |
| `T_Sniper_Human`        | Sniper umano per HumanPlayer						  | `Content/Textures/`    |
| `T_VictoryBackground`   | Texture immagine per schermata vittoria				  | `Content/Textures/`    |
| `BlueButton`		      | Texture per stato Normal per bottone blu			  | `Content/Textures/UI/` |
| `BlueHoveredButton`	  | Texture per stato Hovered per bottone blu			  | `Content/Textures/UI/` |
| `BluePressedButton`	  | Texture per stato Pressed per bottone blu			  | `Content/Textures/UI/` |
| `GreenButton`		      | Texture per stato Normal per bottone verde			  | `Content/Textures/UI/` |
| `GreenHoveredButton`	  | Texture per stato Hovered per bottone verde			  | `Content/Textures/UI/` |
| `GreenPressedButton`	  | Texture per stato Pressed per bottone verde			  | `Content/Textures/UI/` |
| `RedButton`		      | Texture per stato Normal per bottone rosso			  | `Content/Textures/UI/` |
| `RedHoveredButton`	  | Texture per stato Hovered per bottone rosso			  | `Content/Textures/UI/` |
| `RedPressedButton`	  | Texture per stato Pressed per bottone rosso			  | `Content/Textures/UI/` |
| `SliderCircle`		  | Texture per lo stato Normal del cursore dello slider  | `Content/Textures/UI/` |
| `SliderOutlineCircle`	  | Texture per lo stato Hovered del cursore dello slider | `Content/Textures/UI/` |
| `YellowButton`		  | Texture per stato Normal per bottone giallo			  | `Content/Textures/UI/` |
| `YellowHoveredButton`	  | Texture per stato Hovered per bottone giallo		  | `Content/Textures/UI/` |
| `YellowPressedButton`	  | Texture per stato Pressed per bottone giallo		  | `Content/Textures/UI/` |

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
- [UI Pack from Kenney.nl](https://kenney.nl/assets/ui-pack)