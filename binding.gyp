{
  "targets": [
    {
      "target_name": "monero",
      "sources": [
        "src/addon.cc",   
        "src/wallet.cc", 
        "src/walletcallbacks.cc",
        "src/walletargs.cc",
        "src/deferredtask.cc",
        "src/wallettasks.cc",
        "src/pendingtransaction.cc",],
      "libraries": ["../lib/linux/amd64/libwallet_merged.a", 
			      "../lib/linux/amd64/libepee.a", 
			      "../lib/linux/amd64/libeasylogging.a", 
			      "../lib/linux/amd64/liblmdb.a", 
			      "../lib/linux/amd64/libunbound.a", 
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
           "include",
      ],
    }
  ]
}
