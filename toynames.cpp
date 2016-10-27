#include "skylander.h"

//Easier to maintain list of all the ID numbers with the names
//See toydata.xls for all the data that makes this easy to update
//Need to eventually figure out a good way to incorporate the Series / Variant info also.
//Credit for most of the info:
//	https://github.com/reedstrm/SkyReader
//	https://github.com/bettse/SkyReader/tree/more_info
//	https://github.com/Proxmark/proxmark3/blob/master/client/lualibs/default_toys.lua

const char * Skylander::toyName(unsigned short toyID) {
	
	switch (toyID) {
		
		case 0 : return "Whirlwind";                      //0000|0030|regular|air
		case 1 : return "Sonic Boom";                     //0100|0030|regular|air
		case 2 : return "Warnado";                        //0200|0030|regular|air
		case 3 : return "Lightning Rod";                  //0300|0030|regular|air
		case 4 : return "Bash";                           //0400|0030|regular|earth
		case 5 : return "Terrafin";                       //0500|0030|regular|earth
		case 6 : return "Dino-Rang";                      //0600|0030|regular|earth
		case 7 : return "Prism Break";                    //0700|0030|regular|earth
		case 8 : return "Sunburn";                        //0800|0030|regular|fire
		case 9 : return "Eruptor";                        //0900|0030|regular|fire
		case 10 : return "Ignitor";                       //0a00|0030|regular|fire
		case 11 : return "Flameslinger";                  //0b00|0030|regular|fire
		case 12 : return "Zap";                           //0c00|0030|regular|water
		case 13 : return "Wham-Shell";                    //0d00|0030|regular|water
		case 14 : return "Gill Grunt";                    //0e00|0030|regular|water
		case 15 : return "Slam Bam";                      //0f00|0030|regular|water
		case 16 : return "Spyro";                         //1000|0030|regular|magic
		case 17 : return "Voodood";                       //1100|0030|regular|magic
		case 18 : return "Double Trouble";                //1200|0030|regular|magic
		case 19 : return "Trigger Happy";                 //1300|0030|regular|tech
		case 20 : return "Drobot";                        //1400|0030|regular|tech
		case 21 : return "Drill Sergeant";                //1500|0030|regular|tech
		case 22 : return "Boomer";                        //1600|0030|regular|tech
		case 23 : return "Wrecking Ball";                 //1700|0030|regular|magic
		case 24 : return "Camo";                          //1800|0030|regular|life
		case 25 : return "Zook";                          //1900|0030|regular|life
		case 26 : return "Stealth Elf";                   //1a00|0030|regular|life
		case 27 : return "Stump Smash";                   //1b00|0030|regular|life
		case 28 : return "Dark Spyro";                    //1c00|0030|regular|magic
		case 29 : return "Hex";                           //1d00|0030|regular|undead
		case 30 : return "Chop Chop";                     //1e00|0030|regular|undead
		case 31 : return "Ghost Roaster";                 //1f00|0030|regular|undead
		case 32 : return "Cynder";                        //2000|0030|regular|undead
		case 100 : return "Jet Vac";                      //6400|0030|regular|air
		case 101 : return "Swarm";                        //6500|0030|giant|air
		case 102 : return "Crusher";                      //6600|0030|giant|earth
		case 103 : return "Flashwing";                    //6700|0030|regular|earth
		case 104 : return "Hot Head";                     //6800|0030|giant|fire
		case 105 : return "Hot Dog";                      //6900|0030|regular|fire
		case 106 : return "Chill";                        //6a00|0030|regular|water
		case 107 : return "Thumpback";                    //6b00|0030|giant|water
		case 108 : return "Pop Fizz";                     //6c00|0030|regular|magic
		case 109 : return "Ninjini";                      //6d00|0030|giant|magic
		case 110 : return "Bouncer";                      //6e00|0030|giant|tech
		case 111 : return "Sprocket";                     //6f00|0030|regular|tech
		case 112 : return "Tree Rex";                     //7000|0030|giant|life
		case 113 : return "Shroomboom";                   //7100|0030|regular|life
		case 114 : return "Eye-Brawl";                    //7200|0030|giant|undead
		case 115 : return "Fright Rider";                 //7300|0030|regular|undead
		case 200 : return "Anvil Rain";                   //c800|0030|item|none
		case 201 : return "Treasure Chest";               //c900|0030|item|none
		case 202 : return "Healing Elixer";               //ca00|0030|item|none
		case 203 : return "Ghost Swords";                 //cb00|0030|item|none
		case 204 : return "Time Twister";                 //cc00|0030|item|none
		case 205 : return "Sky-Iron Shield";              //cd00|0030|item|none
		case 206 : return "Winged Boots";                 //ce00|0030|item|none
		case 207 : return "Sparx Dragonfly";              //cf00|0030|item|none
		case 208 : return "Dragonfire Cannon";            //d000|0030|item|none
		case 209 : return "Scorpion Striker Catapult";    //d100|0030|item|none
		case 230 : return "Hand Of Fate";                 //e600|0030|item|none
		case 231 : return "Piggy Bank";                   //e700|0030|item|none
		case 232 : return "Rocket Ram";                   //e800|0030|item|none
		case 233 : return "Tiki Speaky";                  //e900|0030|item|none
		case 300 : return "Dragons Peak";                 //2c01|0030|location|none
		case 301 : return "Empire of Ice";                //2d01|0030|location|none
		case 302 : return "Pirate Seas";                  //2e01|0030|location|none
		case 303 : return "Darklight Crypt";              //2f01|0030|location|none
		case 304 : return "Volcanic Vault";               //3001|0030|location|none
		case 305 : return "Mirror Of Mystery";            //3101|0030|location|none
		case 306 : return "Nightmare Express";            //3201|0030|location|none
		case 307 : return "Sunscraper Spire";             //3301|0030|location|light
		case 308 : return "Midnight Museum";              //3401|0030|location|dark
		case 404 : return "Bash";                         //9401|0030|legendary|earth
		case 416 : return "Spyro";                        //a001|0030|legendary|magic
		case 419 : return "Trigger Happy";                //a301|0030|legendary|tech
		case 430 : return "Chop Chop";                    //ae01|0030|legendary|undead
		case 450 : return "Gusto";                        //c201|0030|trapmaster|air
		case 451 : return "Thunderbolt";                  //c301|0030|trapmaster|air
		case 452 : return "Fling Kong";                   //c401|0030|regular|air
		case 453 : return "Blades";                       //c501|0030|regular|air
		case 454 : return "Wallop";                       //c601|0030|trapmaster|earth
		case 455 : return "Head Rush";                    //c701|0030|trapmaster|earth
		case 456 : return "Fist Bump";                    //c801|0030|regular|earth
		case 457 : return "Rocky Roll";                   //c901|0030|regular|earth
		case 458 : return "Wildfire";                     //ca01|0030|trapmaster|fire
		case 459 : return "Ka Boom";                      //cb01|0030|trapmaster|fire
		case 460 : return "Trail Blazer";                 //cc01|0030|regular|fire
		case 461 : return "Torch";                        //cd01|0030|regular|fire
		case 462 : return "Snap Shot";                    //ce01|0030|trapmaster|water
		case 463 : return "Lob Star";                     //cf01|0030|trapmaster|water
		case 464 : return "Flip Wreck";                   //d001|0030|regular|water
		case 465 : return "Echo";                         //d101|0030|regular|water
		case 466 : return "Blastermind";                  //d201|0030|trapmaster|magic
		case 467 : return "Enigma";                       //d301|0030|trapmaster|magic
		case 468 : return "Deja Vu";                      //d401|0030|regular|magic
		case 469 : return "Cobra Cadabra";                //d501|0030|regular|magic
		case 470 : return "Jawbreaker";                   //d601|0030|trapmaster|tech
		case 471 : return "Gearshift";                    //d701|0030|trapmaster|tech
		case 472 : return "Chopper";                      //d801|0030|regular|tech
		case 473 : return "Tread Head";                   //d901|0030|regular|tech
		case 474 : return "Bushwhack";                    //da01|0030|trapmaster|life
		case 475 : return "Tuff Luck";                    //db01|0030|trapmaster|life
		case 476 : return "Food Fight";                   //dc01|0030|regular|life
		case 477 : return "High Five";                    //dd01|0030|regular|life
		case 478 : return "Krypt King";                   //de01|0030|trapmaster|undead
		case 479 : return "Short Cut";                    //df01|0030|trapmaster|undead
		case 480 : return "Bat Spin";                     //e001|0030|regular|undead
		case 481 : return "Funny Bone";                   //e101|0030|regular|undead
		case 482 : return "Knight light";                 //e201|0030|trapmaster|light
		case 483 : return "Spotlight";                    //e301|0030|regular|light
		case 484 : return "Knight Mare";                  //e401|0030|trapmaster|dark
		case 485 : return "Blackout";                     //e501|0030|regular|dark
		case 502 : return "Bop";                          //f601|0030|mini|earth
		case 503 : return "Spry";                         //f701|0030|mini|magic
		case 504 : return "Hijinx";                       //f801|0030|mini|undead
		case 505 : return "Terrabite";                    //f901|0030|mini|earth
		case 506 : return "Breeze";                       //fa01|0030|mini|air
		case 507 : return "Weeruptor";                    //fb01|0030|mini|fire
		case 508 : return "Pet Vac";                      //fc01|0030|mini|air
		case 509 : return "Small Fry";                    //fd01|0030|mini|fire
		case 510 : return "Drobit";                       //fe01|0030|mini|tech
		case 514 : return "Gill Runt";                    //0202|0030|mini|water
		case 519 : return "Trigger Snappy";               //0702|0030|mini|tech
		case 526 : return "Whisper Elf";                  //0e02|0030|mini|life
		case 540 : return "Barkley";                      //1c02|0030|mini|life
		case 541 : return "Thumpling";                    //1d02|0030|mini|water
		case 542 : return "Mini Jini";                    //1e02|0030|mini|magic
		case 543 : return "Eye Small";                    //1f02|0030|mini|undead
		case 1004 : return "Blast Zone";                  //||swapforce|fire
		case 1015 : return "Wash Buckler";                //||swapforce|water
		case 2004 : return "Blast Zone (Head)";           //||swapforce|fire
		case 2015 : return "Wash Buckler (Head)";         //||swapforce|water
		case 3000 : return "Scratch";                     //b80b|0030|regular|air
		case 3001 : return "Pop Thorn";                   //b90b|0030|regular|air
		case 3002 : return "Slobber Tooth";               //ba0b|0030|regular|earth
		case 3003 : return "Scorp";                       //bb0b|0030|regular|earth
		case 3004 : return "Fryno";                       //bc0b|0030|regular|fire
		case 3005 : return "Smolderdash";                 //bd0b|0030|regular|fire
		case 3006 : return "Bumble Blast";                //be0b|0030|regular|life
		case 3007 : return "Zoo Lou";                     //bf0b|0030|regular|life
		case 3008 : return "Dune Bug";                    //c00b|0030|regular|magic
		case 3009 : return "Star Strike";                 //c10b|0030|regular|magic
		case 3010 : return "Countdown";                   //c20b|0030|regular|tech
		case 3011 : return "Wind Up";                     //c30b|0030|regular|tech
		case 3012 : return "Roller Brawl";                //c40b|0030|regular|undead
		case 3013 : return "Grim Creeper";                //c50b|0030|regular|undead
		case 3014 : return "Rip Tide";                    //c60b|0030|regular|water
		case 3015 : return "Punk Shock";                  //c70b|0030|regular|water

		//Default fallback option with toyID
		default		:
			char* toyName = new char[25];
			sprintf(toyName, "zzUNKNOWN_%hu", toyID);
			return toyName;
	}
}
