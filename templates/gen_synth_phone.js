loadedInterfaceName = "template";

interfaceOrientation = "landscape";

pages = [[
{
    "name": "refresh",
    "type": "Button",
    "bounds": [.6, .9, .2, .1],
    "startingValue": 0,
    "isLocal": true,
    "mode": "contact",
    "ontouchstart": "interfaceManager.refreshInterface()",
    "stroke": "#aaa",
    "label": "refrsh",
},
{
    "name": "tabButton",
    "type": "Button",
    "bounds": [.8, .9, .2, .1],
    "mode": "toggle",
    "stroke": "#aaa",
    "isLocal": true,
    "ontouchstart": "if(this.value == this.max) { control.showToolbar(); } else { control.hideToolbar(); }",
    "label": "menu",
},
{
    "name" : "pitchSlider",
    "type" : "Slider",
    "bounds": [0, .05, .15, .9],
    "range": [0,16383], 
    "startingValue": 8192,
    "address" : "/pitch",
    "isVertical" : true,
    "isXFader" : false,
    "ontouchend": "pitchSlider.value = 8192; pitchSlider.draw();"
},
{
    "name" : "modSlider",
    "type" : "Slider",
    "bounds": [.17, .05, .15, .9], 
    "range": [0,127], 
    "startingValue": 63,
    "address" : "/mod",
    "isVertical" : true,
    "isXFader" : false,
},

{
    "name":"cc2Knob",
    "type":"Knob",
    "x":.5, "y":0,
    "width":.25, "height":.25,
    "usesRotation" : false,
    "centerZero" : false,
    "min" : 0, "max" : 127,
    "address" : "/cc2",
},
]

];
