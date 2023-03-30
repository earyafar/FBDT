# Forward and Backward Data Transmission (FBDT)
FBDT is a strategical design for wireless/wired multiRAT data communication. FBDT achieves optimal throughput irrespective of the nature, obstacles, interference etc., of the wireless channel. Simultaneous data transmission across multiple Radio Access Technologies (RATs) such as WiFi and WiGig is a key solution to meet this required capacity demand. However, existing transport layer multi-RAT traffic aggregation schemes suffer from Head-of-Line (HoL) blocking and sub-optimal traffic splitting across RATs, particularly when there is a high fluctuation in channel conditions. As a result, state-of-the-art multi-path TCP (MPTCP) solutions can get overall data rates which are even lower than only using a single WiFi RAT in many practical scenarios, e.g., when the client is mobile. FBDT stream data of any application class such as short-lived, progressive download, live streaming, file download and enable high quality mobile 360deg video streaming by leveraging multiple RATs. We propose the design of FBDT, a novel multi-path transport layer solution that can get the summation of individual rates across RATs despite system dynamics. We have implemented FBDT in Linux kernel/userspace and show substantial improvement in throughput against state-of-the-art schemes, e.g, 2.5x gain in a dual-RAT scenario (WiFi and WiGig) when the client is mobile. 

![image](https://user-images.githubusercontent.com/57430413/228984896-10c176d1-f22d-4c16-9f6f-6ca9b4b6c138.png)
