import subprocess

EXECUTABLE_PATH_TEST = "./ChessLite"
EXECUTABLE_PATH = "/kaggle_simulations/agent/ChessLite"

engine = None

def chess_bot(obs):
    #print(obs.board)
    #print(obs.lastMove)
    global engine

    if engine is None:
        engine = subprocess.Popen(
            EXECUTABLE_PATH_TEST,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        engine.stdin.write(obs.board + '\n')
        engine.stdin.flush()
        return engine.stdout.readline().strip()

    engine.stdin.write(obs.lastMove + ' ' + str(obs.remainingOverageTime) + '\n')
    engine.stdin.flush()
    return engine.stdout.readline().strip()
