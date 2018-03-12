try:
    from Tkinter import *
    import ttk
except ImportError:
    from tkinter import *
    from tkinter import ttk
from module309 import *


def sendBtEvFc():
    "Envoie la commande definie par 'commandSelectCb' et 'dataEn'"
    command = commandSelectCb.get()                         # recuperation de la commande
    dataStr = dataEn.get()                                  # recuperation du parametre (en string)
    if dataStr.isdigit():                                   # si le parametre est un nombre
        data = int(dataStr)                                     # on le traduit
    else:                                                   # sinon
        data = 0                                                # on le choisit nul
    if command==commandList[0]:                             # Verifie si le parametre est dans
        if (data>127) or (data<-128):                       # l'intervalle autorise pour la commande
            data = 0                                        # si non, on le choisit nul
    if command==commandList[1] or command==commandList[2]:  # Verifie si le parametre est dans
        if (data>127) or (data<0):                          # l'intervalle autorise pour la commande
            data = 0                                        # si non, on le choisit nul
    if data!=0:                                             # si le parametre n'est pas nul
        f = createFrame(command,data)                           # on construit la trame de bit
        s = createSignal(f)                                     # puis le signal audio
        playSignal(s)                                           # et on le joue
#        messageDisplay.config(test=f)
        print(f)
    else:                                                   # sinon
        dataEn.delete(0, END)                               # on efface le champ d'entree
        dataEn.insert(0,'0')                                # pour y mettre '0'



def stopBtEvFc():
    print(commandList)
    f = createFrame(commandList[3],0)                       # on construit la trame de bit
    s = createSignal(f)                                     # puis le signal audio
    playSignal(s)                                           # et on le joue


# MAIN PROGRAM
###############
# Fenetre principale
fen = Tk()                                                  # cree la fenetre principale
fen.title('Emetteur FSK pour ELEC-H-309')                   # definit le titre de la fenetre
# Choix de la commande
commandSelectCb = ttk.Combobox(fen)                         # cree la combo box pour la selection de la commande
commandSelectCb['values'] = commandList[0:3]                # assigne la liste des commandes a la combo box
commandSelectCb.set(commandList[0])                         # definit la commande par defaut
commandSelectCb['state'] = 'readonly'                       # rend la liste non editable
commandSelectCb.grid(row=0,column=0)                        # affiche le widget
# Entree du parametre
dataEn = ttk.Entry(fen)                                     # cree le champ d'entree pour le parametre de la commande
dataEn.insert(0,'10')                                       # initialise le parametre
dataEn.grid(row=0,column=1)                                 # affiche le widget
# Bouton d'envoi
sendBt = ttk.Button(fen)                                    # cree le bouton d'envoi
sendBt.config(text="Go!")                                   # definit le texte du bouton
sendBt.config(command=sendBtEvFc)                           # definit la fonction associee au bouton
sendBt.grid(row=1,column=0)                                 # affiche le bouton
# Affichage de la trame
#messageDisplay = ttk.Label(fen)
#messageDisplay.pack()
# affiche la fenetre (et bloque l'execution du programme)
fen.mainloop()
