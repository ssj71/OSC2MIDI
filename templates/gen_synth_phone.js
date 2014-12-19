loadedInterfaceName = "generic synth (phone)";

interfaceOrientation = "landscape";

function oct(dir)//int dir)
{
    //octSlider.setValue(dir);
    //octLabel.setValue(dir);
    
    if(dir == 1 && octSlider.value < 4)
    {
        octSlider.setValue(octSlider.value +1);
    }
    else if(dir == -1 && octSlider.value > -4)
    {
        octSlider.setValue(octSlider.value -1);
    }
   
    switch(octSlider.value)
    {
        case -4:
            octLabel.setValue(-4);
            octDnButton.setValue(octDnButton.max);
            octUpButton.setValue(0);
            break;
        case -3:
            octLabel.setValue(-3);
            octDnButton.setValue(octDnButton.max);
            octUpButton.setValue(0);
            break;
        case -2:
            octLabel.setValue(-2);
            octDnButton.setValue(octDnButton.max);
            octUpButton.setValue(0);
            break;
        case -1:
            octLabel.setValue(-1);
            octDnButton.setValue(octDnButton.max);
            octUpButton.setValue(0);
            break;
        case 0:
            octLabel.setValue(0);
            octDnButton.setValue(0);
            octUpButton.setValue(0);
            break;
        case 1:
            octLabel.setValue(1);
            octUpButton.setValue(octUpButton.max);
            octDnButton.setValue(0);
            break;
        case 2:
            octLabel.setValue(2);
            octUpButton.setValue(octUpButton.max);
            octDnButton.setValue(0);
            break;
        case 3:
            octLabel.setValue(3);
            octUpButton.setValue(octUpButton.max);
            octDnButton.setValue(0);
            break;
        case 4:
            octLabel.setValue(4);
            octUpButton.setValue(octUpButton.max);
            octDnButton.setValue(0);
            break;
    }
    
}

control.octave = oct;

pages = [[
{
    "name": "refresh",
    "type": "Button",
    "bounds": [.6, .9, .14, .1],
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
    "bounds": [.95, .0, .05, .2],
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
    "range": [0.1,16383.1], 
    "startingValue": 8192.1,
    "address" : "/pitch",
    "isVertical" : true,
    "isXFader" : false,
    "ontouchend": "pitchSlider.setValue(8192.1);"
},
{
    "name" : "modSlider",
    "type" : "Slider",
    "bounds": [.17, .05, .15, .9], 
    "range": [0.1,127.1], 
    "startingValue": 63.1,
    "address" : "/midi/mod",
    "isVertical" : true,
    "isXFader" : false,
},

{
    "name" : "cc2Slider",
    "type" : "Slider",
    "bounds": [.35, .05, .14, .65], 
    "range": [0.1,127.1], 
    "startingValue": 0.1,
    "address" : "/midi/cc2",
    "isVertical" : true,
    "isXFader" : false,
},
{
    "name" : "cc3Slider",
    "type" : "Slider",
    "bounds": [.51, .05, .15, .65], 
    "range": [0.1,127.1], 
    "startingValue": 0.1,
    "address" : "/midi/cc3",
    "isVertical" : true,
    "isXFader" : false,
},
{
    "name" : "cc4Slider",
    "type" : "Slider",
    "bounds": [.68, .05, .15, .65], 
    "range": [0.1,127.1], 
    "startingValue": 0.1,
    "address" : "/midi/cc4",
    "isVertical" : true,
    "isXFader" : false,
},
{
    "name" : "cc5Slider",
    "type" : "Slider",
    "bounds": [.85, .05, .14, .65], 
    "range": [0.1,127.1], 
    "startingValue": 0.1,
    "address" : "/midi/cc5",
    "isVertical" : true,
    "isXFader" : false,
},

{
    "name" : "octLabel",
    "type" : "Label",
    "bounds": [.6, .75, 0.14, 0.15], 
    "value" : "0",
},
{
    "name" : "octSlider",
    "type" : "Slider",
    "bounds": [.63, .75, 0.0, 0.0], 
    "range": [-4,4], 
    "startingValue": 0,
    "address" : "/octave",
    "isVertical" : true,
    "isXFader" : false,
},
{
    "name": "octDnButton",
    "type": "Button",
    "bounds": [.35, .75, .25, .25],
    "mode": "toggle",
    "stroke": "#aaa",
    "isLocal": true,
    "ontouchstart": "control.octave(-1)",
    "label": "Oct--",
},
{
    "name": "octUpButton",
    "type": "Button",
    "bounds": [.74, .75, .25, .25],
    "mode": "toggle",
    "stroke": "#aaa",
    "isLocal": true,
    "ontouchstart": "control.octave(1)",
    "label": "Oct++",
},

]

];
