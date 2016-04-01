{
  "targets": [
    {
      "target_name": "fslink",
      "sources": [ "fslink.cc" ],
        "include_dirs" : [
          "<!(node -e \"require('nan')\")"
        ]
    }
  ]
}
