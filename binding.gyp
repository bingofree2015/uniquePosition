{
    'targets': [
        {
            'target_name': 'uniquePosition',
            'defines': ['V8_DEPRECATION_WARNINGS=1'],
            'sources': ['src/uniquePosition.cc','src/base64.cpp'],
            "conditions": [
                [
                    "OS=='win'", {
                        "configurations": {
                            "Release": {
                                "msvs_settings": {
                                    "VCCLCompilerTool": {
                                        "RuntimeLibrary": 0,
                                        "Optimization": 3,
                                        "FavorSizeOrSpeed": 1,
                                        "InlineFunctionExpansion": 2,
                                        "WholeProgramOptimization": "true",
                                        "OmitFramePointers": "true",
                                        "EnableFunctionLevelLinking": "true",
                                        "EnableIntrinsicFunctions": "true",
                                        "RuntimeTypeInfo": "false",
                                        "PreprocessorDefinitions": [
                                            "WIN32_LEAN_AND_MEAN"
                                        ],
                                        "ExceptionHandling": "0",
                                        "AdditionalOptions": [
                                            "/EHsc"
                                        ]
                                    },
                                    "VCLibrarianTool": {
                                        "AdditionalOptions": [
                                            "/LTCG"
                                        ]
                                    },
                                    "VCLinkerTool": {
                                        "LinkTimeCodeGeneration": 1,
                                        "OptimizeReferences": 2,
                                        "EnableCOMDATFolding": 2,
                                        "LinkIncremental": 1
                                    }
                                }
                            },
                            "Debug": {
                                "msvs_settings": {
                                    "VCCLCompilerTool": {
                                        "PreprocessorDefinitions": [
                                            "WIN32_LEAN_AND_MEAN"
                                        ],
                                        "ExceptionHandling": "0",
                                        "AdditionalOptions": [
                                            "/EHsc"
                                        ]
                                    },
                                    "VCLibrarianTool": {
                                        "AdditionalOptions": [
                                            "/LTCG"
                                        ]
                                    },
                                    "VCLinkerTool": {
                                        "LinkTimeCodeGeneration": 1,
                                        "LinkIncremental": 1
                                    }
                                }
                            }
                        }
                    }
                ],
            ],

            'include_dirs': [
                "<!(node -e \"require('nan')\")"
            ]
        }
    ]
}
