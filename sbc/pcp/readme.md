Dedicated build in ash for pCP. The extension files must be owned by root. The extension should be built as a user account.

To build:
```bash
sudo chown root.root hypnic -R
mksquashfs hypnic hypnic.tcz -b 4k -no-xattrs
```

This has no architecture constraints, so can run on any architecture. Leverages pCP tools (pcp bs)
