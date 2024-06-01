import wandb
from run_item import run_item

def train() -> None:
    wandb.init()
    config = wandb.config
    run_item(config)

if __name__ == "__main__":
    train()
