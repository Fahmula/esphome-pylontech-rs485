# Notes on issues with Voltronic Inverter

I let copilot figure out why this component is not working with my Voltronic
Axpert VM IV inverter. Turns out that this component is not answering to the
inverter's request for the version of the protocol used. The packets do seem
to match the pylontech spec.

This "Get version" command however is not documented under the V3.5 docs for
some reason, so I looked around for other versions of the spec and it seems
like the latest version that describes this command and its response is V3.3.

Copilot noted this, made a script to parse the logs and summarize findings but
also added some notes about stuff not related to only implementing command 0x4F.
Maybe use these hints as improvements. I'd have to take a closer look to examine
the legitimacy of their concerns. For now I'll just implement the 0x4F command.

I also had to do an AI transcription for the protocol specs, as that's easier
for these LLMs to read.
