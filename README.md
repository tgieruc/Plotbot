# PLOTBOT
## Projet robotique BA6

Ce projet a été conçu dans le cadre du cours "Systèmes embarqués et Robotique" à l'EPFL.

Le rapport est disponible ici : [Rapport PDF](https://github.com/tgieruc/Plotbot-MTBA6/blob/main/G38_Rapport_Miniprojet_Plotbot.pdf).

### Instructions :
* Placer le dossier *PLACE_NEXT_TO_lib* à coté du dossier lib de l'e-puck
* Ouvrir le dossier Miniprojet avec Eclipse
* Placer les quatres fichiers .wav dans une carte microSD, l'inserer dans l'e-puck

#### Important
Changer _lib\e-puck2_main-processor\src\sensors\VL53L0X\VL53L0X.c_ ligne 36  `VL53L0X_configAccuracy(&device, VL53L0X_LONG_RANGE);` en
`VL53L0X_configAccuracy(&device, VL53L0X_DEFAULT_MODE);`


### Processing app
>Le fichier Processing *path_generator.pde* est à ouvrir avec Processing.<br />
>Prérequis : Android Mode, Sound Library, Android Studio

### Web app (obsolète)
https://editor.p5js.org/tgieruc/sketches/f2lFWcYWd
