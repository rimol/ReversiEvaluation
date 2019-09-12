# ReversiEvaluation

評価関数を生成するためのコードが主に置かれています。
privateにするかも。

## 特徴

評価関数で使うパターンの形を固定にするのではなく、自由に変えられるようにしています。これによって、パターンを変えてみたりする実験がしやすいと思われますが、自分はLogistelloが使っているパターンしか使ってません（完）実際、パターンを変えても総パラメータ数が変わらなければ、強さはそんなに変わらない感じがします（適当）。ここにこだわるよりは、教師データを充実させるほうが効果がたかそうです。

## 完全読み性能（2019/09/11更新）

[The FFO endgame test suite](http://www.radagast.se/othello/ffotest.html)を解くのにかかった時間など。
反復深化を実装するために容量制限付きのstd::unordered_mapっぽいものを作る必要があったため、面倒くさくなって反復深化は実装していません。
-これを頑張ればもう少し探索ノードを減らせて高速化できそうですが、もう飽きました-
以下のテストの実行には評価値データが別に必要ですが、GitHubにはあげてません。

### テスト実行環境

Intel Core i5-8259U
並列処理は実装してないので、必然的に1スレッド実行になります。

### 結果

Depth: 探索深さ（空きマスの数）
BestScore: 最善スコア
ScoreLockTime: 最善スコアが求まるまでにかかった時間。
WholeTime: 最善手順まで求めるのにかかった時間。
Nodes: 最善スコアを求めるまでに探索したノード数。（手順を求める時に増える分はカウントしてない）
NPS: Nodes Per Second
Moves: 最善手順（複数ある場合はそのうちの1つ）
M: 10^6
G: 10^9

|  #  | Depth | BestScore | ScoreLockTime(sec) | WholeTime(sec) | Nodes | NPS | Moves |
|:---:|:-----:|:---------:|:------------------:|:--------------:|:-----:|:---:|:-----:|
| 40 | 20 | 38 | 0.732 | 0.742 | 24.7M | 33.7M | A2 B1 C1 B6 C7 A7 B7 F8 E8 C6 D7 A8 B8 A6 G8 D8 G7 H8 C8 F7 |
| 41 | 22 | 0 | 1.425 | 1.819 | 37.7M | 26.4M | H4 A3 A2 G6 G5 G7 F8 H6 H3 B7 G1 H1 A5 H5 H8 H2 H7 E8 A8 A1 B2 A7 |
| 42 | 22 | 6 | 3.722 | 3.865 | 121M | 32.7M | G2 H1 C2 G1 F1 D2 B2 A2 B6 C6 B5 G8 H8 A7 A4 A6 B8 A1 B1 A8 B7 C7 |
| 43 | 23 | -12 | 6.194 | 6.628 | 148M | 24.0M | C7 H4 H5 B8 B7 A8 A7 A5 H3 H6 G3 A4 A3 H2 G7 A2 B1 A1 B2 G2 H1 H7 H8 |
| 44 | 23 | -14 | 1.949 | 2.87 | 46.0M | 23.6M | B8 G5 D2 A3 B7 A8 A7 C8 G7 G8 H6 H5 G6 F1 D1 H1 G2 H7 H8 B2 A2 B1 A1 |
| 45 | 24 | 6 | 33.602 | 54.524 | 941M | 28.0M | B2 C1 G5 H6 G4 H3 B1 A6 A8 G3 D8 B8 B7 C8 G2 A1 H5 H4 G8 G7 H8 H7 H2 H1 |
| 46 | 24 | -8 | 7.38 | 8.386 | 181M | 24.6M | B3 H3 H2 C1 B7 G7 A5 A4 B6 B5 H8 H7 F8 A6 A2 G1 B2 A7 A8 A3 G2 H1 B1 A1 |
| 47 | 25 | 4 | 27.08 | 27.964 | 748M | 27.6M | G2 B8 B7 B2 A5 G3 A1 H1 A2 A3 H3 G1 G6 G5 G7 A8 A7 H7 G4 H8 G8 H5 H6 H4 H2 |
| 48 | 25 | 28 | 41.068 | 52.977 | 1.06G | 25.9M | F6 G5 G3 F2 H4 H5 H3 G6 G2 A7 H6 G7 D1 C1 B1 H7 H8 B7 A8 B2 A1 E1 G1 H2 H1 |
| 49 | 26 | 16 | 72.265 | 88.92 | 2.16G | 29.9M | E1 H4 G6 H6 H5 F8 G7 G4 G8 H3 B2 B1 H7 H8 A1 G1 A2 B7 A8 A7 D8 C7 B8 H2 H1 G2 |
| 50 | 26 | 10 | 117.486 | 128.079 | 2.98G | 25.3M | D8 E8 G8 H8 F2 G7 B7 A7 A6 A5 A2 A3 A8 B8 A4 G2 H1 F1 H7 D1 B2 A1 H2 G1 C1 B1 |
| 51 | 27 | 6 | 40.705 | 51.849 | 909M | 22.3M | E2 H2 G7 F1 H1 H3 D1 C8 D8 G2 B7 D2 C1 C2 B2 A2 B8 H8 G8 A8 A3 B3 A7 A5 A1 B1 C3 |
| 52 | 27 | 0 | 46.022 | 53.286 | 1.01G | 22.0M | A3 B3 C2 A2 F2 E8 B7 F7 D8 E7 F8 A8 B8 C8 A7 C1 A1 E1 G1 F1 B1 B2 G2 G8 G7 H8 H1 |
| 53 | 28 | -2 | 350.079 | 399.254 | 8.63G | 24.6M | D8 B6 D7 C2 A5 A3 B4 G7 D1 C1 H8 G8 H2 E8 G2 A7 A6 A4 C8 B7 A8 B8 A2 B2 A1 H1 G1 B1 |
| 54 | 28 | -2 | 433.632 | 580.51 | 10.4G | 24.0M | C7 G6 G5 F6 G4 C8 B7 E2 B8 H6 H5 H4 B1 A8 A7 A1 G2 H1 F2 G1 F1 G8 E8 F8 H8 H2 G7 H7  |
| 55 | 29 | 0 | 3335.37 | 3494.94 | 78.5G | 23.5M | B7 F8 G6 F2 H5 H6 D2 C8 H4 G4 A3 A8 E2 F1 C1 H2 E1 H7 B8 D1 G7 B2 G1 H8 G8 G2 H1 A1 B1  |
| 56 | 29 | 2 | 446.731 | 512.808 | 8.22G | 18.4M | H5 H4 H3 D8 C8 C7 E8 G8 G4 A4 G3 H6 H7 G7 H8 F8 B2 B7 G2 A2 A8 A5 B8 A7 A3 H1 H2 B1 A1 |
| 57 | 30 | -10 | 950.978 | 977.064 | 19.6G | 20.6M | A6 B5 B4 A3 A4 E2 F2 E1 G1 B3 G2 C3 C2 D2 B2 A5 C1 A7 H2 H1 D1 G7 H8 F1 G8 A1 B1 A2 A8 B7 |
| 58 | 30 | 4 | 226.664 | 315.641 | 5.20G | 22.9M | G1 F2 H3 D8 C8 D7 E8 H5 H6 H7 H2 G6 A3 G8 F8 B8 A5 G2 B7 A8 A7 B2 A4 G7 H8 B1 H1 A2 H4 A1 |
| 59 | 34 | 64 | 2575.47 | 3241.42 | 89.2G | 34.6M | G8 E8 A5 B8 A8 A7 B7 A4 B5 B4 G3 H4 F3 E3 D3 C3 A3 B3 G2 F2 E2 G1 D2 C2 H2 H1 D1 B2 F1 B1 A2 E1 C1 A1 |

総主要時間: 2時間47分

### Raw Data

上の内容は以下の生データの手入力なので（クソ）、間違いがあれば教えてください。

  A B C D E F G H
1 O * * O O O O X
2 * O O O O O O X
3 O O X X O O O X
4 O O X O O O X X
5 O O O O O O X X
6 *   * O O O O X
7     * * O * * X
8       *        
Color:Black
Black: 12, White: 32
Depth:20
BestScore:38
ScoreLockTime:0.732 sec
WholeTime:0.742 sec
Nodes:24729917
NPS:33784039
A2 B1 C1 B6 C7 A7 B7 F8 E8 C6 D7 A8 B8 A6 G8 D8 G7 H8 C8 F7 
  A B C D E F G H
1   O O O O O *  
2 * * O O O O X  
3   O O O O O O *
4 X X X X X O O *
5   X X O O X *  
6 O O X O X X    
7 * * O X X O *  
8   O O O   * O  
Color:Black
Black: 14, White: 28
Depth:22
BestScore:0
ScoreLockTime:1.425 sec
WholeTime:1.819 sec
Nodes:37754489
NPS:26494378
H4 A3 A2 G6 G5 G7 F8 H6 H3 B7 G1 H1 A5 H5 H8 H2 H7 E8 A8 A1 B2 A7 
  A B C D E F G H
1     O O O      
2   * * * X X * O
3 O O O O O X O O
4 * O O O O X O O
5 X * O O O X X O
6   * * O O X O O
7     * O O O X O
8     O O O O    
Color:Black
Black: 9, White: 33
Depth:22
BestScore:6
ScoreLockTime:3.722 sec
WholeTime:3.865 sec
Nodes:121859229
NPS:32740254
G2 H1 C2 G1 F1 D2 B2 A2 B6 C6 B5 G8 H8 A7 A4 A6 B8 A1 B1 A8 B7 C7 
  A B C D E F G H
1   * O O O O O  
2     O O O O *  
3   X X X O O *  
4   X X O O O O *
5   X X O O O X  
6 X X X X O X X  
7     * O X O *  
8     O O O O O  
Color:Black
Black: 15, White: 26
Depth:23
BestScore:-12
ScoreLockTime:6.194 sec
WholeTime:6.628 sec
Nodes:148721807
NPS:24010624
C7 H4 H5 B8 B7 A8 A7 A5 H3 H6 G3 A4 A3 H2 G7 A2 B1 A1 B2 G2 H1 H7 H8 
  A B C D E F G H
1     X   O * X  
2     X * O X * X
3   X X O O O X X
4 X X X X O O O X
5 X X X X O O *  
6 O O X X O X *  
7 * * O O O O    
8   * * O O O *  
Color:Black
Black: 21, White: 20
Depth:23
BestScore:-14
ScoreLockTime:1.949 sec
WholeTime:2.87 sec
Nodes:46056439
NPS:23630805
B8 G5 D2 A3 B7 A8 A7 C8 G7 G8 H6 H5 G6 F1 D1 H1 G2 H7 H8 B2 A2 B1 A1 
  A B C D E F G H
1       X X X X  
2 X * X X X O *  
3 X X O X O O *  
4 X X X O X O *  
5 X X O X X O *  
6 * O X X X O O *
7 O * O O O O * *
8   * * * O O *  
Color:Black
Black: 22, White: 18
Depth:24
BestScore:6
ScoreLockTime:33.602 sec
WholeTime:54.524 sec
Nodes:941875023
NPS:28030326
B2 C1 G5 H6 G4 H3 B1 A6 A8 G3 D8 B8 B7 C8 G2 A1 H5 H4 G8 G7 H8 H7 H2 H1 
  A B C D E F G H
1   * * X X X    
2   * O O O X    
3 * * O O O X X  
4 * O O O O X X X
5 * * O O O O X X
6   * O X O X X X
7   * X X O O *  
8   X X X X * O  
Color:Black
Black: 21, White: 19
Depth:24
BestScore:-8
ScoreLockTime:7.38 sec
WholeTime:8.386 sec
Nodes:181734813
NPS:24625313
B3 H3 H2 C1 B7 G7 A5 A4 B6 B5 H8 H7 F8 A6 A2 G1 B2 A7 A8 A3 G2 H1 B1 A1 
  A B C D E F G H
1   X X X X X    
2     X X X X *  
3   X X X X O *  
4 O O O O O O *  
5 * X O X X O *  
6 X X X O X O *  
7     X X O O *  
8     O O O O *  
Color:Black
Black: 22, White: 17
Depth:25
BestScore:4
ScoreLockTime:27.08 sec
WholeTime:27.964 sec
Nodes:748940372
NPS:27656586
G2 B8 B7 B2 A5 G3 A1 H1 A2 A3 H3 G1 G6 G5 G7 A8 A7 H7 G4 H8 G8 H5 H6 H4 H2 
  A B C D E F G H
1   * * * * O    
2 O * O O O      
3 O O O O X X * *
4 O X O X X O O *
5 O X X O O O * *
6 O X X O O * *  
7     X X X O *  
8   O O O O O O  
Color:Black
Black: 12, White: 27
Depth:25
BestScore:28
ScoreLockTime:41.068 sec
WholeTime:52.977 sec
Nodes:1066109895
NPS:25959625
F6 G5 G3 F2 H4 H5 H3 G6 G2 A7 H6 G7 D1 C1 B1 H7 H8 B7 A8 B2 A1 E1 G1 H2 H1 
  A B C D E F G H
1   * O X * O    
2 * * X X O O *  
3 O O O O O X X  
4 O O O O O X    
5 O O O X O X X  
6 O O O O X X    
7   * * O O X    
8     X * O      
Color:Black
Black: 13, White: 25
Depth:26
BestScore:16
ScoreLockTime:72.265 sec
WholeTime:88.92 sec
Nodes:2165180104
NPS:29961670
E1 H4 G6 H6 H5 F8 G7 G4 G8 H3 B2 B1 H7 H8 A1 G1 A2 B7 A8 A7 D8 C7 B8 H2 H1 G2 
  A B C D E F G H
1         X      
2 * * X X X   * *
3 * O O O X O O O
4 * O O O X O O O
5 * O X O X O X O
6 * O O X X O O O
7 * * O O X O * *
8   * O *   O *  
Color:Black
Black: 12, White: 26
Depth:26
BestScore:10
ScoreLockTime:117.486 sec
WholeTime:128.079 sec
Nodes:2983921412
NPS:25398102
D8 E8 G8 H8 F2 G7 B7 A7 A6 A5 A2 A3 A8 B8 A4 G2 H1 F1 H7 D1 B2 A1 H2 G1 C1 B1 
  A B C D E F G H
1         X * O  
2     * * * O *  
3 * * * O O O X  
4 X O O O O O X X
5 * O O X X O X X
6 O O X O O O X X
7     X X X X * X
8         X X    
Color:Black
Black: 19, White: 18
Depth:27
BestScore:6
ScoreLockTime:40.705 sec
WholeTime:51.849 sec
Nodes:909765619
NPS:22350217
E2 H2 G7 F1 H1 H3 D1 C8 D8 G2 B7 D2 C1 C2 B2 A2 B8 H8 G8 A8 A3 B3 A7 A5 A1 B1 C3 
  A B C D E F G H
1       O * *    
2   *   X O * * O
3 * * O X X O O O
4 O O O X O O O O
5 O O O X X O O O
6 O O O X X X O O
7 * * O X       O
8   *            
Color:Black
Black: 10, White: 27
Depth:27
BestScore:0
ScoreLockTime:46.022 sec
WholeTime:53.286 sec
Nodes:1014386347
NPS:22041335
A3 B3 C2 A2 F2 E8 B7 F7 D8 E7 F8 A8 B8 C8 A7 C1 A1 E1 G1 F1 B1 B2 G2 G8 G7 H8 H1 
  A B C D E F G H
1     * * O O *  
2       O O O * *
3   X X X X O O O
4     X X O O X O
5   X X X X X O O
6   * O O O X O O
7   * X * O X * O
8       * * X    
Color:Black
Black: 16, White: 20
Depth:28
BestScore:-2
ScoreLockTime:350.079 sec
WholeTime:399.254 sec
Nodes:8638347708
NPS:24675423
D8 B6 D7 C2 A5 A3 B4 G7 D1 C1 H8 G8 H2 E8 G2 A7 A6 A4 C8 B7 A8 B8 A2 B2 A1 H1 G1 B1 
  A B C D E F G H
1   * O O O      
2 X X O O * *   *
3 X X X X O O O O
4 X X X X O X    
5 X X X O X X    
6 X X O O O *    
7     * O O O    
8     * O * * *  
Color:Black
Black: 18, White: 18
Depth:28
BestScore:-2
ScoreLockTime:433.632 sec
WholeTime:580.51 sec
Nodes:10443260285
NPS:24083232
C7 G6 G5 F6 G4 C8 B7 E2 B8 H6 H5 H4 B1 A8 A7 A1 G2 H1 F2 G1 F1 G8 E8 F8 H8 H2 G7 H7 
  A B C D E F G H
1     * *        
2 O * O * * *    
3 * O O O O X X X
4 X X O X O O * *
5 X X X O O O O *
6 X X O O O O *  
7 X * X X X O *  
8       X X *    
Color:Black
Black: 17, White: 18
Depth:29
BestScore:0
ScoreLockTime:3335.37 sec
WholeTime:3494.94 sec
Nodes:78559929661
NPS:23553587
B7 F8 G6 F2 H5 H6 D2 C8 H4 G4 A3 A8 E2 F1 C1 H2 E1 H7 B8 D1 G7 B2 G1 H8 G8 G2 H1 A1 B1 
  A B C D E F G H
1   * O O O O O  
2     O O O O *  
3   X X X O O *  
4   X X O X O *  
5   X O O O O O *
6 X X X X X O X  
7       X O O *  
8         * * *  
Color:Black
Black: 14, White: 21
Depth:29
BestScore:2
ScoreLockTime:446.731 sec
WholeTime:512.808 sec
Nodes:8220873331
NPS:18402289
H5 H4 H3 D8 C8 C7 E8 G8 G4 A4 G3 H6 H7 G7 H8 F8 B2 B7 G2 A2 A8 A5 B8 A7 A3 H1 H2 B1 A1 
  A B C D E F G H
1                
2           * *  
3       X X O O O
4     X X X O O O
5   * X X O X O O
6 * O O O X X X O
7 * * O X O O * O
8   O O O O O *  
Color:Black
Black: 12, White: 22
Depth:30
BestScore:-10
ScoreLockTime:950.978 sec
WholeTime:977.064 sec
Nodes:19617901785
NPS:20629185
A6 B5 B4 A3 A4 E2 F2 E1 G1 B3 G2 C3 C2 D2 B2 A5 C1 A7 H2 H1 D1 G7 H8 F1 G8 A1 B1 A2 A8 B7 
  A B C D E F G H
1   * X O O O *  
2 * * O O O * * *
3 * O O O X O O *
4 * O O O O X O *
5 * O X O X X X  
6 O O X X X X    
7 *   X   X X    
8                
Color:Black
Black: 14, White: 20
Depth:30
BestScore:4
ScoreLockTime:226.664 sec
WholeTime:315.641 sec
Nodes:5207793043
NPS:22975827
G1 F2 H3 D8 C8 D7 E8 H5 H6 H7 H2 G6 A3 G8 F8 B8 A5 G2 B7 A8 A7 B2 A4 G7 H8 B1 H1 A2 H4 A1 
  A B C D E F G H
1                
2                
3   * * * * * * O
4     O O O O O *
5 * * O O O O O X
6 O O O O X X X X
7     X X O O X X
8     X X * O * X
Color:Black
Black: 12, White: 18
Depth:34
BestScore:64
ScoreLockTime:2575.47 sec
WholeTime:3241.42 sec
Nodes:89205147641
NPS:34636386
G8 E8 A5 B8 A8 A7 B7 A4 B5 B4 G3 H4 F3 E3 D3 C3 A3 B3 G2 F2 E2 G1 D2 C2 H2 H1 D1 B2 F1 B1 A2 E1 C1 A1 
Done!
10014.9 sec elapsed.
