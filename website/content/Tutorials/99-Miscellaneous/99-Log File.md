import YoutubeVideo from "@site/src/components/YoutubeVideo"
import OsTab from "@site/src/components/OsTab"

If you experience a crash or another problem in Coollab, you can send us the log file to help us debugging the issue. Here is how to find it:

- Open the launcher, in the menu select "<span class="icon-rocket"></span> Commands > <span class="icon-file-text2"></span> Reveal Coollab logs in explorer"
- Copy the "LOGS Coollab.txt" file and send it to us!
- **NB:** the log file is reset each time you open Coollab. So if you opened it again after encountering your problem, you will need to reproduce the problem to make sure the log file is up-to-date


In case you can't even open the launcher, here is where you will find the log files:

<OsTab>
  <TabItem value="windows" label="Windows">

```
%appdata%/Coollab
```

  </TabItem>
  <TabItem value="linux" label="Linux">

```
~/.local/share/Coollab
```    

  </TabItem>
  <TabItem value="mac" label="MacOS">

```
~/Library/Application Support/Coollab
```

  </TabItem>
</OsTab>

<YoutubeVideo id="CgKGrkSBOR0"/>
<br/>