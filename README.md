# gladys-esp8266-Witty
Module ESP8266 Witty + extend card MCP23017 + card 8 relays + card 8 input

# Module ESP8266 Witty

Nous allons utiliser un module ESP8266 Witty pour gérer 8 relais et 8 entrées
Matériel nécessaire :
- module ESP8266 [lien ebay](https://www.ebay.fr/itm/ESP8266-Serial-WIFI-Witty-Cloud-Development-Board-ESP-12F-Module-MINI-Nodemcu/264039039222?_trkparms=aid=111001&algo=REC.SEED&ao=1&asc=20160908105057&meid=7912a8f239d74abcb7126c140473364d&pid=100675&rk=6&rkt=15&sd=173503702813&itm=264039039222&_trksid=p2481888.c100675.m4236&_trkparms=pageci:8df836f6-4f16-11e9-aa46-74dbd1802d22%7Cparentrq:b59144011690ad794a1eef74fff9554d%7Ciid:1)
- une carte Optocoupler Isolation Voltage [lien ebay](https://www.ebay.fr/itm/Optocoupler-Isolation-Voltage-Test-Board-8-Channel-AC-220V-MCU-TTL-for-PLC/263162567047?_trkparms=aid=111001&algo=REC.SEED&ao=1&asc=20160908105057&meid=7912a8f239d74abcb7126c140473364d&pid=100675&rk=4&rkt=15&sd=173503702813&itm=263162567047&_trksid=p2481888.c100675.m4236&_trkparms=pageci:8df836f6-4f16-11e9-aa46-74dbd1802d22%7Cparentrq:b59144011690ad794a1eef74fff9554d%7Ciid:1)
- une carte 8 relais avec optocoupleur [lien ebay](https://www.ebay.fr/itm/1-2-4-8-16-12V-Channel-Relay-Module-With-optocoupler-For-PIC-AVR-DSP-ARM-Arduino/173604730014?var=472283979123&_trkparms=aid=222007&algo=SIM.MBE&ao=2&asc=20170831090034&meid=e1b7924e5ee348e0948b592fde06df41&pid=100005&rk=1&rkt=9&sd=391975892322&itm=472283979123&_trksid=p2047675.c100005.m1851) 
- une carte extension mcp23017

Avec tout ça, nous allons contrôler des lampes raccordées à des télérupteurs. 
Le câblage existant est conservé. 
La carte relais sera relié aux télérupteurs.
La carte "Optocoupler isolation 8 entrées" sera reliée aux lampes pour visualiser leurs états.

# Description

Téléversez le fichier ino ou bin dans l'ESP

## Raccordement mcp23017

Le port A du mcp23017 est utilisé pour commander les relais
Le port B est utilisé pour contrôler l'état des lampes

Je vous recommande de raccorder l'I2C comme suite:

| pin |      mcp23017      |     ESP8266
|-----|--------------------|--------------------
|I2C SDA | I2C SDA | pin GPIO5
|I2C SCL | I2C SCL | pin GPIO0
|Interrupt | INT B | pin GPIO14

## Visualisation 
Le module esp est équipé d'une led 3 couleurs, nous allons l'utiliser pour visualiser chaque étape.
> **led clignotante bleue :**
> Le module doit être configuré

 > **led verte clignotante :**
 >Indique la lecture du fichier de configuration

>**led verte clignotante (rapide) :**
>mode 'debug' activé

>**led fixe blanche :**
>le module est démarré, il attend les commandes et évènements

> **led blanche clignotante :**
> le module envoie des données à Gladys

> **led rouge clignotante :**
> indique une erreur
