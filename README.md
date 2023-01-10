### ELSYS PROSJEKT
Hei gruppe 10!
Under her kommer jeg til å legge inn en liten introduksjon til hvordan git kan brukes, så får vi se om det er tilstrekkelig...
Vi får ta en liten felles gjennomgang fortløpende, så blir dette gull:)
Informasjonen merker jeg skriver nå kanskje blir litt kaotisk, men prøver med det jeg husker og finner av informasjon.
Dette er bare en start, så får vi se hvor mye mer som trenges fortløpende bare:)

Denne siden her er fin hvis det er noe du lurer på som kanskje ikke står under:

[Intro til git](https://www.atlassian.com/git/tutorials/learn-git-with-bitbucket-cloud "Google's Homepage")

Kan anbefales å lese denne uansett egentlig, f.eks. om du plutselig kjeder deg en lørdagskveld...

## Hvilke verktøy "trenger" vi?
Egentlig kan man kjøre alt av git-kommandoer fra terminal (enten direkte på PC eller i VS Code eller en hvilken som helst annen terminal). Men av egen erfaring kan det være like greit, kanskje litt lettere i starten også, å bruke programmer som er laget for akkurat det her. Under her er en link til skrivebordsprogrammer som skal gjøre det enklere for oss å gjøre endringer på giten:

[Git verktøy](https://git-scm.com/downloads)

Da får du en pakke med et par programmer som kan brukes

Jeg ønsker at alle lager en mappe lokalt på sin egen PC hvor vi kan klone giten (dvs. laste ned alt på giten slik at du kan lokalt gjøre endringer i filene og senere laste opp til den nettbaserte giten igjen. NB!!!: Ikke på OneDrive - det skaper trøbbel
Det finnes flere metoder for å klone, men den jeg personlig synes er lettest er å bruke Git BASH

# Git BASH
**Alt som står under her kan egentlig gjøres i alle andre terminaler, så lenge du har riktig mappevei/bane (kan vise hvordan dette gjøres også**


**Hente fra git**
Er vel egentlig bare en egen Git-terminal?
Dette er for Windows, det er dessverre alt jeg kan atm.
1. For å klone går du bare inn i mappen du har laget (IKKE ONEDRIVE!), høyreklikk, velg  "Vis flere alterntiver", velg Git Bash
2. Deretter går du inn i giten på nett, trykker på code, og kopierer den linken som heter HTTPS
3. Inne i Git Bash skriver du følgende:
```
git clone https://github.com/SanderBakke/ELSYS-PROSJEKT
```
for å lime inn i terminal høyreklikker du og velger "paste", Ctrl+C brukes for å avbryte kjørende kommando

4. Enter
Da skal du ha alle filene lokalt på egen PC.

Hvis det har blitt lagt til flere filer på giten som du ikke har i mappen din får du de enkelt med følgende kommando i Git Bash (Åpne Git Bash i respektiv mappe, likt som forklart over)
```
git pull
```

**Laste opp til git**
For å laste opp filer til giten må du bruke følgende kommandoer:
```
git add .
```
```
git commit -m "meldingen du vil skrive til endringene du har gjort"
```
```
git push
```


**Når du vil fjerne endringer du har gjort lokalt på din PC**
```
git stash
```
```
git config user.name <brukernavn>
```
```
git config user.email <email>
```

# Git GUI
Kan også brukes til å klone
Kan sikkert skrive en kortet ned versjon senere, men denne nettsiden forklarer ganske greit hvordan Git GUI brukes

[Git GUI](https://www.geeksforgeeks.org/working-on-git-for-gui/)

