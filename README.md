# gladys-esp8266-Witty
Module ESP8266 Witty + extend card MCP23017 + card 8 relays + card 8 input
# Module ESP8266 Witty
![ESP8266WITTY](data:image/jpeg;base64,/9j/4AAQSkZJRgABAQAAAQABAAD/2wCEAAkGBxATEhUUExQWFhQWGSAbGBgYGRcfIRsgIB0iIiAdHx8kKDQsJCYxJx8fLTIkMSsuMDAwIys1QD8uNzQuMC4BCgoKDg0OFRAPFTcZFhk3Ky03NysrKzcrKy0rNy03ODArNzc3MSs3NzgzKystNzQ1MSstNysrKy04ODEtKysrK//AABEIAJYAlgMBIgACEQEDEQH/xAAcAAABBAMBAAAAAAAAAAAAAAAAAQUGBwIDBAj/xAA+EAABAwIEAwYDBgQEBwAAAAABAgMRACEEEjFBBQZREyJhcYGRMkLwByNSocHRYpKx4RQzcoIVJCVDU7Kz/8QAGQEBAAMBAQAAAAAAAAAAAAAAAAECAwQF/8QAJBEAAgICAgIBBQEAAAAAAAAAAAECESExAxIyUUEEEyJhcfD/2gAMAwEAAhEDEQA/ALxooooAooooAooooAoopKAWiuIcRbJUAScpgkC07id43jSult1KtDUWiaZsopKKkgWikooBaKSigFopKKAWikooBaSlooBKKWigEopawWoAEkwBqTQGVRfEcwtvOrZZX3G/81aZvPypI/NXt1DLzZzQHGyELUjDk5c6R33juETokbqOulQ5sOKQEYZxLiQZKAMrlxe2+uozVjLkWkax4/llsMhOUZIyiwAiPKh58IGYwkAT09arrhvNakLWFkoShP8A3CErVG1hCjrY30po47zJiOIOBtgFLIM/64vmVNsu8G3XworZekWVwznnDrUQZCdUrucw65Yny1kX0qTYTFtupCm1pWk7pINUgzjXmHC2jKHJJK20hz5YNlCRaZg26Vnw/iymG1BtThVGYLbNkqnVW8QTZSbxWqk0ZuKZedFQLB89Fppo4iFlRyqyRm0srLoRttUr4ZxrDvj7tYmJymyo6xV1JMq4tDlRRRUlQooooAoopaASlpKKAKWkrl4jjm2Wy44oJSPoADcnpUMG195KElSyEpSJJOgHWq15x5iD6cilrZZV8KUpla/41iRCOidTrXJzlxhWKPZqeDA1SyqfQuqFgr+HQU0t9o2n/nMq2QIQFEKUoxbs1AyB1MxXPPkvxf8Av0bxhWzXgWVtSr/ENnDfNcKCv4eyVfMfEetNvZf4h+GG8gUrupBJyjqT+da+HcPXiHMjSdT1skdSasvgvB2sM3CbqPxLjX9h4UdRy9ll+X8MsBw1CEQv7xZTC1LuVec7eFN+O5VwykHskhpRIIUJgek2p7srUextWb1k7E1ipNZRo4rRCXODYzDShhGcrF3UphQ6pBmw3pqZfbw6UqbKXHlpMqIs30AB38fDprZCVWEKJP1tXDjeEMupjKEHNmC2wkHMN5itY8qeyj4/RBW2G2QVvpHb5gQkABXWV7e4k11pDzobeWruIzBvKjKqbkAJnvEnoo048R5UcL4WPvGwLha+9YaWGmkR7imFbSsS4Q4oMtJKiAqAEDdKRYTFaYZTKJhwTm/El8tthTjIIH3iYUmdp/fpUu4dzVhnVluShxJIUlYjLBg301Ma1U3/ABBbykoZbASlMd5SpgblQMgDwNz1sKzaxrbKylnIu6p7cRMpKYzAwet48zVk2irSZek0VTnL3M7uFlslRcUiW0qzKBKVaDbKQScwMCKmfB/tBwjpCXD2SupMoJ3hX7xV0yjjRMKKRKgRIuDRUlRawccCQSogAak1nUQ58xBHZIJUEnMZHURY+/5mok6Vlox7Oh2xnMuFQkkqJIFkhJlXgKrrFcxnGFSg4lvEAkNtuWSgadw6Fw/iPpFc5KnFGQSALgEGBsJ+by06zTTzHwXt1qVh1ZXUps2bhyLkA7EAj+0VhJuWDo+2oZFxzxCsmNYIV/5EgJX5/hX9XrgVhmi6hCX05FfOoKSET+IHfyMeNM7HM+IQ2WHCQg7KAIH+mdPMU6cIwJfC190oQO8SoCJ39NZ0HjpRLqreCvlotHhPD22GwlqCDcqsc3jO/wDSug2IAiZ1moVwjjDgyZPu2BMAJ+M7JEg2/mVr6SbA8badClLSUIBIC1EDMBqYmYFczk/k3S9DqgXuI8RWSiSZB06ftWDZCkgpIUjYgz+dbFKG396EGiE7j1H6itqQDexG0belYlQJ0n8jWcAd4nS99vOiJYpgCSYCb38vyFQnjXFEPuqbXmSw0QpRuM5Nhf5RBJG/lNseZuPB3tGiVNtIglyTfQwAPn6IJ6kxFonnLCQpWR9TqCUpJzZVK+ZW2aOsx51pEoxwxfEEEOIYGVkrkT8RjxmY/eteAxIaSpRblahCFKFhcTA9/qZb+BcNSpzM8tIURuYGosOgHva2tSrjWNDCVtBQczRlBAhA/wBPlEelhbN0awYkYS+sAgKISdQCYPppUx5K5SU6oOOCE6gH+p/QU0cPwSGUpexAur/KbOqjEyR0q2OR3y5g23CBmUVzA6LUB+QA9Kny1oh4HvDshCQkaCittFXoyObGuKS2pSRmUEkgGbmPCqi5yx+PGTEIGdYQEuJAJAIMkxrl8tN6uWuDHcJaduRCvxJsf71ElZeEurKEw3PLa4S404lZsQ3cK9JB9L084FbwcOJW2tOVs9iyUntFEiCopFwgTqf2FSzmrl/EpT3MUpubBSSJPgRr7Gojh8A7hC8tSHH0lIAyklSjIlTk3mJiJA03JONJGzm5IZcJwYvNrWvusp1WRaeg8frWBWfYtOdm2kDCMDVQClZyOpiTofAX1NO2DzYhhanQtxIXDcqgixUq95gAahVD2BdxSULStKGBZKT3csWMJk5t4ve+l6s6eyitaNAxSnlhTrRTgWQAV2TmCdMxAk3+UaTFprPCuB9tx6QMOlQF9SdlET0+UZvBJvXLinVOlDDbYATqkBQBULEmTMeJiJ8q28VQw4622twttoF0sIhOeIIF7z1joNyazfH6LqfsdcHxp6UurUpDcWAsJOmbXuxoPCw1p+wHMSVoU46ns2we6TrH8QixM6T5Coe/iX1uF7FNZcKyYCSQCYEAFMSo/Wk1jg32nkKfUtKWkkJQggT1zWABN/hTlneAL4uDX6NVKyyBiGynPmGSJBNo8b3qGc1cbLpcaIysJgKWDJnXKBuu1hMC5NNvFnXFtdo8VJSRISQk/EDBgRBPUgARaaZ3+JlkM9m4FulopSkIs1mkSDbvdVASSdxVooiTDE8QDIRlUlalIV2bcZuyz7kyJXa6iPypw5ewTzraW3XQAokoBygqkiQnwkD1sPmjk4fhXXkpQEBTiRmUruiOtzEC/XYdKd8ZxdxDfZOMoCrFJhJAvYjWRbYxboIrZKv6ZN2ZM49eCzswhxKrnYjaCRI2mL663rS92WHScZixGYy0wPmPgNhXXwfhQRGIxIMTIB23zrnbT1pi52xCMQjtnGloaBLbD2U3VcnPY907ACbedTdsPCI8xxl7FYwOOmTeANEiDYCvRH2dH/p7Pmv/AOiq80cNQWXAtQlMEBSSCCY0kb+GtWTyDzRj0lLbMLQST2arpAJuQdU6++x31wjKmy86KwbJIBIgxpSVYoZ0UtNHMfH2cG0XHTf5UDVZ6D99qAZvtFxJabad1AXlXAnuka+4FRrgvEGnUryrCiFEi94gHTpqKifMfOuJfS6VobOb4RHwCZgfVz7VGOFvnLqZBPn1/WuPn4HyXUurOiE+qV5LQ4lwNl5IKxBN8yTBnx2PrTFxjhuNS2GkBK2UpgwCF/HmEiepIkX1ri4fzO8232ZAWkCBOo9al2G41h3smVcKUYymxv8AW1cv3ufgxyx7R9o1qE/F0xix3FUIbSw2pDq+4hY72Y2mQrUQbQfatmIwDWGOVkheIUYTniMuhCdpm14m4G8vHEOEpWsGBmFwrcRpB/TTwpiVwp1p9L61KeCFFUWB6wNAb+RPSuri5ociuDM5Ra2jga4WUAuYoFAMlKVC7hHhIMXpewTi1hb2ZvDtCE5AAB0TMAeZtoB0FdGFx5xDxL6VJaQ1mcbXPdITBKRqiTBgRW7iDa3XXG2UpbThxKmpBbUBqrNvPQ+9bb2V0Rni/FXXFEKStSUghnvZgnpnn8zW7gfBCRnczEG0xdROwsbeh8Aa6uJYZptKU3U4oZioEwJ28RYwRrM+FduG5ncS0psoQZECwjSLpiDbaw8KKNLAcreTmx2EXhVjI5Co0BEgWgGLEHoelwLU78OwuWMTjFSY+7RAmAJnL4XMVzNNN4ZH+KxhlapLbajdStb/AFbeohxTmFx3EdsYJFgDmjKZ7pE6QYI3vNQ84Gif4aMS2p7Ekt4NBk3lThmyE9elvGOtQHnDmJeNdCCCyw0crbAHwRaSLSrr0ro41zG5ig0lw5EtABCGxlSiLFQE/EdfDStqeAPkrS0hl1agg9pnzFJUcxXn0JkGZvGgIE1KpEPIz4LCPPOjDNtWAskn4ZiXVEfNp6d0CvQHIfKKMGymRLkSSdZ6n9tvOtP2fclN4JvMRmdVdSjqT1PTwG3nU0rRIzb+ApaSlqxUSoxzfym3jIUT30iBt7H95FSeioaJTooLjvJ+JYJGUqT5X9tD6VGezCDAEXuK9QPMpUIUAR0NQ/mLkDDv95I7w0uQfRQv6GaimiykmUiFVsQ4RBBII0p847yZi8OswgrRtHxe2/pPlUcC/wArVGydEk4bzQ+2sFZ7RIEQdfeJ96lXCuNsPrIBykgQFQCesdarQKrNKq4+X6Hjm+0fxl7RrHma3lFmY/hSHkFKgCBbUgjyIuPSPGaacNwp3CgnDhK5UmQtWU5UqkiYggxew8Jpj4NzE8xI+NJMkKn8jtUr4RzBh3UALUltY1CjHsawc/qODzXePtF6hPWGQzGviC2gQgKKjrdXUA6DoPfwcWkM4FoYnFXUf8prcnrH1HnArt4rzHgEKU6Wi52d0Lgd87Cek/MQevSav5g429i3e0dImISBokTMCvQg3NJ1SMXg6eI8XVjHC6+QVmyU3CUjYAVk1hZygGVKMAQfCNOtNOFQVSI0GYnYRufrenzAoVmSEiQVRlEmbjQ3jWrPBVZO7CcBdLikOAt9mYcJHw9ABuTsN/K9XTyHywGUBa0ZfwN/hHVXVZ1J8ugA0ck8skfePDRUoSQbTook3JiNzEVPEiKRV7EnWELRRRWhmFLSUUAtFJS0AUUUlAa32UrEKAIOxFRDmPkDDvyUjKo77jyV+hmpnS1DVkptHn3jfJ2Kwx+EuDwEGPKe9/tnyqPg3r06+ylYyqAUDsRUQ5j5Bw+I7ybKixm48lfoZFRlFk0yk0mswafuN8m4rDqgIK07ERPtv6TUfT/S1LJMeLIK2SkXUYgdbiovhsGtayiII+LNYJA1KugFPXG3e7l6Cmtp99wBoFSgogBIuVEfCnrF7DShBmAXClhhJUFGBa7h6nw6Db3NXl9mX2eIwyUvPAKdI9B4Dw8d/KtX2X/Z8lhIfeAU4oeYH8I8Op3001tIClWQ3QUUUVYqFFFFAFLSUtAJRS0UAlFLRQCUtFFAFFFFAa3mUrEKAUDsRNRLj/IWGflSRlXGu/8AN+81MaKhqyU6POPN32e41peZKe1T0AhXtv6H0FTf7M/s8DIDz4lwj+UfgSf/AGV/tG82o42FCCJHjQlAAgCAKiiewqUgCBYUtFFWKhRRRQBRRRQBRRS0AUUUUAUUUUAUUUUAUUUUAUUUUAlFFFAFFFFAFFFFAFFFFAFFFFAf/9k=)

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

Téléversez le fichier ino ou le bin dans l'ESP

## Raccordement mcp23017

Le port A du mcp23017 est utilisé pour commander les relais
Le port B est utilisé pour contrôler l'état des lampes

Je vous recommande de raccorder l'I2C comme ci-dessous, mais pas d’inquiétude vous pourrez rechercher comment est raccordé votre mcp23017 (voir §I2C scanner) 

| pin |      mcp23017      |     ESP8266
|-----|--------------------|--------------------
|I2C SDA | I2C SDA | pin GPIO5
|I2C SCL | I2C SCL | pin GPIO0
|Interrupt | INT B | pin GPIO14

## Visualisation 
Le module ESP est équipé d'une led 3 couleurs, nous allons l'utiliser pour visualiser chaque étape.
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
 
## Actions
Le bouton de l'ESP va nous permettre d'effectuer différentes actions
> **un appui court :**
> va nous permettre d'activer le mode debug

> **deux appuis courts après avoir activé le mode debug :**
> imprime sur le port série la configuration

> **un appui long (8sec) :**
> réinitialisation, active le mode configuration. Utile lors d'un changement de paramètre.

## Première étape
Configurez Gladys
* installez le module (si ce n'est pas déjà fait) de **Mathieu**, [Gladys-Device-HTTP](https://github.com/MathieuAndrade/Gladys-Device-HTTP)
* Ajoutez un nouveau device, comme indiqué, dans le champ identifier renseignez l'adresse complète de votre device (exemple http://192.168.0.50/)

## Initialisation
A l'allumage le module clignote en bleue, il faut le configurer.
C'est très simple, prenez votre smartphone, 
* connectez vous sur le réseau wifi ESP suivi de chiffre.
* Allez sur l'url "*http://192.168.4.1*", vous êtes sur la page d'accueil du module.
* Cliquez sur "*Configurer*".
* Renseignez les paramètres demandés
* Cliquez sur "*Sauvegarder*"
* Retournez à la page d'accueil
* Fermez la page

La led bleue clignotante passe au fixe, puis le module redémarre.
S'il ne redémarre pas tout seul, débranchez l'alimentation puis rebranchez.

La led verte clignote puis passe au blanc clignotant. 
Le module envoie l'état des lampes à Gladys.
