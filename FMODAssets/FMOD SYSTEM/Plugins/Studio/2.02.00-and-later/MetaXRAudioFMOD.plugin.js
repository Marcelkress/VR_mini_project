studio.plugins.registerPluginDescription("MetaXRAudio Spatializer", {
    companyName: "Meta",
    productName: "MetaXRAudio Source",
    parameters: {
        "Acoustics": { displayName: "Enable Acoustics" },
        "Reverb Send": { displayName: "Reverb Send" },
        "HRTF Intensity": { displayName: "HRTF Intensity" },
        "V. Radius": { displayName: "Volumetric Radius" },
        "Refl. Send": { displayName: "Early Reflections Send" },
        "Directivity": { displayName: "Directivity Pattern",  enumeration: ["None", "Human Voice"], },
        "Dtv. Intensity": { displayName: "Directivity Intensity" },
        "Direct Enabled": { displayName: "Direct Enabled" },
        "Reverb Reach": { displayName: "Reverb Reach" },
        "Occl. Intensity": { displayName: "Occlusion Intensity" },
        "Medium Absrp.": { displayName: "Medium Absorption" },
    },

    deckUi: {
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.HBoxLayout,
        spacing: 8,
        items: [
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.HBoxLayout,
                contentsMargins: { left: 6, right: 6 },
                spacing: 2,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Layout,
                        layout: studio.ui.layoutType.GridLayout,
                        contentsMargins: { left: 6, right: 6 },
                        spacing: 2,
                        items: [
                            {
                                deckWidgetType: studio.ui.deckWidgetType.DistanceRolloffGraph,
                                row: 0,
                                column: 0,
                                columnSpan: 2,
                                minimumDistanceBinding: 'Atten. Min',
                                maximumDistanceBinding: 'Atten. Max',
                                rolloffTypeBinding: 'Atten. Mode',
                                rolloffTypes: {
                                0: studio.project.distanceRolloffType.LinearSquared,
                                1: studio.project.distanceRolloffType.Linear,
                                2: studio.project.distanceRolloffType.Inverse,
                                3: studio.project.distanceRolloffType.InverseTapered,
                                4: studio.project.distanceRolloffType.Custom,
                                },
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Button,
                                binding: "Acoustics",
                                row: 1,
                                column: 0,
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Dial,
                                binding: "Reverb Send",
                                row: 1,
                                column: 1,
                            },
                        ],
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Layout,
                        layout: studio.ui.layoutType.HBoxLayout,
                        contentsMargins: { left: 0, right: 14 },
                        isFramed: true,
                        items: [
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.VBoxLayout,
                                contentsMargins: { left: 6, right: 10 },
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Label,
                                        text: "Experimental Controls:",
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Pixmap,
                                        filePath: __dirname + "/MetaLogo.png",
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dropdown, binding: "Directivity",
                                    },
                                ],
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.VBoxLayout,
                                contentsMargins: { left: 6, right: 10 },
                                spacing: 14,
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial, binding: "Dtv. Intensity",
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Button, binding: "Medium Absrp.",
                                    },
                                ],
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.GridLayout,
                                contentsMargins: { left: 30, right: 30 },
                                spacing: 14,
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial, binding: "Refl. Send",
                                        row: 0,
                                        column: 0,
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial, binding: "V. Radius",
                                        row: 1,
                                        column: 0,
                                    },


                                ],
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.VBoxLayout,
                                contentsMargins: { left: 6, right: 10 },
                                spacing: 14,
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial, binding: "HRTF Intensity",
                                        row: 1,
                                        column: 1,
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Button, binding: "Direct Enabled",
                                    },
                                ],
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.VBoxLayout,
                                contentsMargins: { left: 6, right: 10 },
                                spacing: 14,
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial, binding: "Reverb Reach",
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial, binding: "Occl. Intensity",
                                    },
                                ],
                            },
                        ],
                    },
                ],
            },
            { deckWidgetType: studio.ui.deckWidgetType.OutputMeter, },
        ],
    },
});

studio.plugins.registerPluginDescription("MetaXRAudio Ambisonics", {
    companyName: "Meta",
    productName: "MetaXRAudio Ambisonics",
    parameters: {},

    deckUi: {
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.VBoxLayout,
        contentsMargins: { left: 6, right: 6, top: 6 },
        spacing: 40,
        items: [
            { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + "/MetaLogo.png", },
        ],
    }
});


studio.plugins.registerPluginDescription("MetaXRAudio Reflections", {
    companyName: "Meta",
    productName: "MetaXRAudio Reflections",
    parameters: {
        "Early Refl.": { displayName: "Early Reflections Enabled" },
        "Reverb Enabled":  { displayName: "Reverb Enabled" },
        "Reverb Level": { displayName: "Reverb Level" },
        "Voice Limit":  { displayName: "Voice Limit" },
    },

    deckUi: {
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.HBoxLayout,
        contentsMargins: { left: 6, right: 6 },
        spacing: 12,
        items: [
        { deckWidgetType: studio.ui.deckWidgetType.Pixmap, filePath: __dirname + "/MetaLogo.png" },
        {
            deckWidgetType: studio.ui.deckWidgetType.Layout,
            layout: studio.ui.layoutType.HBoxLayout,
            contentsMargins: { left: 0, right: 14 },
            spacing: 8,
            items: [
                {
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.VBoxLayout,
                    contentsMargins: { left: 0, right: 20 },
                    spacing: 14,
                    items: [
                        { deckWidgetType: studio.ui.deckWidgetType.Button, binding: "Early Refl.", },
                        { deckWidgetType: studio.ui.deckWidgetType.Button, binding: "Reverb Enabled", },
                    ]
                },
                {
                    deckWidgetType: studio.ui.deckWidgetType.Layout,
                    layout: studio.ui.layoutType.VBoxLayout,
                    contentsMargins: { left: 0, right: 0 },
                    spacing: 14,
                    items: [
                        { deckWidgetType: studio.ui.deckWidgetType.Dial, binding: "Reverb Level", },
                        { deckWidgetType: studio.ui.deckWidgetType.NumberBox, binding: "Voice Limit", },
                    ]
                },
            ],
        },
        { deckWidgetType: studio.ui.deckWidgetType.OutputMeter, },
        ],
    }
});
