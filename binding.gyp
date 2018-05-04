{
  "conditions": [
    ['OS=="linux"', {
      "variables": {
        'boost_libraries': [
          "-lboost_serialization", 
          "-lboost_thread", 
          "-lboost_system", 
          "-lboost_date_time", 
          "-lboost_filesystem", 
          "-lboost_chrono", 
          "-lboost_program_options", 
          "-lboost_regex",
        ]
      }
    }],
    ['OS=="mac"', {
      "variables": {
        'boost_libraries': [
          "-lboost_serialization-mt", 
          "-lboost_thread-mt", 
          "-lboost_system-mt", 
          "-lboost_date_time-mt", 
          "-lboost_filesystem-mt", 
          "-lboost_chrono-mt", 
          "-lboost_program_options-mt", 
          "-lboost_regex-mt",
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
            "../lib/libwallet_merged.a", 
            "../lib/libepee.a", 
            "../lib/libeasylogging.a", 
            "../lib/liblmdb.a", 
            "../lib/libunbound.a", 
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
            "../lib/libwallet_merged.a", 
			      "../lib/libepee.a", 
			      "../lib/libeasylogging.a", 
			      "../lib/liblmdb.a", 
			      "../lib/libunbound.a", 
            "<@(boost_libraries)",
			      "-lssl",
            "-lcrypto",
            "-lz",
            "-lpcsclite"],
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
