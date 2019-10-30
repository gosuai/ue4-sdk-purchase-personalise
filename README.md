# contents:
* [prerequisites](#prerequisites)
* [platforms supported](#platforms-supported)
* [how it works](#how-it-works)
* [most used scenarios](#most-used-scenarios) 
* [how to use](#how-to-use)
* [demo](#demo)
* [install and setup](#install-and-setup)
* [ue4 nodes and data](#ue4-nodes-and-data)


***
**GosuAI Purchase Personalisation SDK** enables Unreal Engine developer to personalise player's in-game shopping experience.
***

## prerequisites:
* Unreal Engine 4.22+
* win 7 (or higher) or macos 10.9.2 (or higher)
* Visual Studio 2017 (or higher) or XCode 10.1 (or higher)
* Make sure to acquire API credentials before going live with the plugin. You will need `app_id` and `secret_key`. [Do this for now](mailto:slava.smirnov@gosu.ai) to receive them.


## platforms supported:
initial release:
* `Steam`
* `Epic Games Store`
* `Google Play`
* `iOS`

coming soon:
* ps4
* xbox
* switch


## how it works:
GosuAI Purchase Personalisation receives players' data (e.g. player_id, platform, sessions and in-shop behaviour), expects per `player_id` prediction request and provides a list of in-game shop items (with their categories) sorted by probability purchase for that particular `player_id`. Resulted list of personalised items is therefore available for preferred scenario.


## most used scenarios:
(may combine) 
* show new **recommended** category with in-game shop items relevant to that specific player (think of it as a new tab)
* visually **highlight** items relevant to that specific player (think of it as a new visual effect)
* show **popup/loading screen** with a specific item out of those relevant to that specific player (think of it as a new in-game shop promotional approach

As items include their corresponding categories it's easy to optionally filter them according to either game logic or business needs.

For example you may want to exclude items from some categories out of recommendations as they only make sense when presented on specific conditions or separate tabs.

Another case might be that you have an objective to drive purchases for specific categories of items and lower others. Creating new tab and filtering recommendations by category is one way to do it.

Pro tip: useful tactic is to setup recommended tab as a default tab (when player enters the shop) and rotate some items within this tab on daily basis.


## how to use:
* install plugin with github or [UE4 marketplace](https://www.unrealengine.com/marketplace/en-US/store) (preferred)
* acquire gosu.ai credentials [here](mailto:slava.smirnov@gosu.ai)
* pick 1 of 3 scenarios you would use personalised recommendations for:
  * recommended tab with items
  * highlight recommended item
  * popup with recommended item
* make sure you have design layouts (UMG) being prepared for scenario
* setup plugin:
  * submit credentials at plugin settings (this is critical before going live)
  * make sure you have setup all the data
  * receive personalised items (filter some categories if needed)
* connect `get predictions` function with UMG and enjoy providing your players with personalised shopping experience


## demo:
text


## install and setup
* text for marketplace
* text for github


## ue4 nodes and data
describe nodes and their parameters
