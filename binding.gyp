{
  "targets": [
    {
      "target_name": "build_monero",
      "type": "none",
      "actions": [
        {
          "action_name": "retrieve_from_github",
          "variables": {
            "monero-branch": "release-v0.12-multisig-wallet-assembled"
          },
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
			      "-lboost_serialization", 
			      "-lboost_thread", 
			      "-lboost_system", 
			      "-lboost_date_time", 
			      "-lboost_filesystem", 
			      "-lboost_chrono", 
			      "-lboost_program_options", 
			      "-lboost_regex",
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
