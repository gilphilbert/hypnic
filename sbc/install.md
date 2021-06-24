cp hypnic-poweroff.sh /usr/lib/systemd/system-shutdown/
cp hypnic.conf /etc/
cp hypnic.service /usr/lib/systemd/system/
cp hypnic-service.py /usr/bin/
systemctl daemon-reload
systemctl enable hypnic.service
systemctl start hypnic.service


wget -O - https://raw.github.com/luismartingil/commands/master/101_remote2local_wireshark.sh | sudo bash
