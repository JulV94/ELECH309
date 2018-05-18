try:
    import pyaudio
except ImportError:
    import PyAudio
from numpy import int16,linspace,sin,pi, hstack


commandDico = {'Avance':[0,0], 'Droite':[0,1], 'Gauche':[1,0]}
commandList = list(commandDico.keys())

# L'amplitude max du signal est de 1
def playSignal(signal, sampleRate=44100):
    "Cette fonction joue le signal audio passe en parametre"
    # instantiate PyAudio (1)
    p = pyaudio.PyAudio()
    # open stream (2)
    stream = p.open(format=pyaudio.paInt16,
                    channels=1,
                    rate=sampleRate,
                    output=True)
    # play stream (3)
    data = (32767*signal).astype(int16).tostring()
    stream.write(data)
    # stop stream (4)
    stream.stop_stream()
    stream.close()
    # close PyAudio (5)
    p.terminate()


def createFrame(command, data):
    "Cette fonction cree la trame audio correspondant a la commande voulue"
    message = commandDico[command]
    s = bin(data)
    message = message + [0]*(10-len(s))
    for a in list(s[2:]):
        message = message + [int(a)]
    parity = message.count(1) % 2
    frame = [0] + message + [parity, 0, 0]
    return frame

def createSignal(frame, sampleRate=44100, fc=1000, df=100, Tb=0.1):
    "Cette fonction cree le signal module correspondant a la trame donnee"
    t = linspace(0, Tb, sampleRate*Tb, False)   # cree le vecteur des instants d'echantillonnage pour un symbole
    sig0 = sin(2*pi*(fc-df)*t)                  # cree le vecteur des echantillons du symbole '0'
    sig1 = sin(2*pi*(fc+df)*t)                  # cree le vecteur des echantillons du symbole '1'
    sig = [sig0, sig1]
    signal = sig[frame[0]]
    for a in frame[1:]:
        signal = hstack((signal,sig[a]))
    return signal
