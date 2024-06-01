def _flatten_dict_gen(d: dict | list, parent_key: str, sep: str) -> dict:
    if isinstance(d, dict):
        for k, v in d.items():
            new_key = parent_key + sep + k if parent_key else k
            if isinstance(v, dict) or isinstance(v, list):
                yield from flatten_dict(v, new_key, sep=sep).items()
            else:
                yield new_key, v
    elif isinstance(d, list):
        for i, v in enumerate(d):
            new_key = parent_key + sep + str(i) if parent_key else str(i)
            if isinstance(v, dict) or isinstance(v, list):
                yield from flatten_dict(v, new_key, sep=sep).items()
            else:
                yield new_key, v

def flatten_dict(d: dict | list, parent_key: str = '', sep: str = '.') -> dict:
    return dict(_flatten_dict_gen(d, parent_key, sep))
