{
  "conditions": [
    ['OS=="linux"', {
      "variables": {
        'additional_libraries': [
          "-lpcsclite",
        ]
      }
    }],
    ['OS=="mac"', {
      "variables": {
        'additional_libraries': [
          "-framework PCSC",
        ]
      }
    }]
  ],
  "targets": [
    {
      "target_name": "build_monero",
      "type": "none",
      "actions": [
        {
          "action_name": "retrieve_from_github",
          "inputs": "",
          "outputs": [
            "../deps/libwallet_merged.a", 
            "../deps/libepee.a", 
            "../deps/libeasylogging.a", 
            "../deps/liblmdb.a", 
            "../deps/libunbound.a", 
          ],
          "action": ["./build.sh"],
          "message": "Building monero libraries",
        },
      ],
    },
    {
      "target_name": "monero",
      "dependencies": ["build_monero"],
      "sources": [
        "src/addon.cc",   
        "src/wallet.cc", 
        "src/walletcallbacks.cc",
        "src/walletargs.cc",
        "src/deferredtask.cc",
        "src/wallettasks.cc",
        "src/pendingtransaction.cc",],
      "libraries": [
            "../deps/libwallet_merged.a", 
			      "../deps/libepee.a", 
			      "../deps/libeasylogging.a", 
			      "../deps/liblmdb.a", 
			      "../deps/libunbound.a", 
            "../deps/libboost_serialization.a", 
            "../deps/libboost_thread.a", 
            "../deps/libboost_system.a", 
            "../deps/libboost_date_time.a", 
            "../deps/libboost_filesystem.a", 
            "../deps/libboost_chrono.a", 
            "../deps/libboost_program_options.a", 
            "../deps/libboost_regex.a",
            "<@(additional_libraries)",
			      "-lssl",
            "-lcrypto",
            "-lz",
            ""],
      "include_dirs": [
           "include"
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
