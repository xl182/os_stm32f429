import os
import re
import yaml
import logging
import hashlib
from pathlib import Path


def init_logger(config):
    log_config = config.get("logging", {})
    logging.basicConfig(
        level=getattr(logging, log_config.get("level", "INFO")),
        format=log_config.get("format",
                              "%(asctime)s - %(levelname)s - %(message)s"),
        handlers=[
            logging.StreamHandler(),
            logging.FileHandler(log_config.get("file", "./tool/text_processor.log"))
        ])
    return logging.getLogger("TextProcessor")


def generate_op_id(file_path, op):
    op_str = f"{file_path}-{op['type']}-{op['pattern']}"
    return hashlib.md5(op_str.encode()).hexdigest()


def process_file(file_path, operations, executed_ops):
    logger = logging.getLogger("TextProcessor")

    if not Path(file_path).exists():
        logger.error(f"File not found: {file_path}, current work dir: {os.getcwd()}")
        return False

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        if content.endswith("// modified"):
            logger.info(f"Skipping modified file: {file_path}")
            return False
    except Exception as e:
        logger.error(f"Error reading {file_path}: {str(e)}")
        return False

    original_content = content
    modified = False

    for op in operations:
        op_id = generate_op_id(file_path, op)

        if op_id in executed_ops:
            logger.info(f"Skipping executed operation: {op_id[:8]}...")
            continue
        
        try:
            if op["type"] == "replace":
                new_content, count = re.subn(op["pattern"], op["replacement"],
                                             content)
                if count > 0:
                    content = new_content
                    logger.info(
                        f"Replaced pattern '{op['pattern']}' in {file_path}")
                    modified = True
                else:
                    logger.warning(
                        f"Pattern '{op['pattern']}' not found in {file_path}")

            elif op["type"] == "insert_after":
                new_content = ""
                last_index = 0
                found = False
                
                for match in re.finditer(op["pattern"], content):
                    start, end = match.span()
                    new_content += content[last_index:end] + op["insert_text"]
                    last_index = end
                    found = True
                
                if found:
                    new_content += content[last_index:]
                    content = new_content
                    logger.info(
                        f"Inserted text after pattern '{op['pattern']}' in {file_path}"
                    )
                    modified = True
                else:
                    logger.warning(
                        f"Pattern '{op['pattern']}' not found in {file_path}")


        except re.error as e:
            logger.error(f"Regex error: {str(e)}")
            print(op["pattern"])
        except Exception as e:
            logger.error(f"Processing error: {str(e)}")

    if modified and content != original_content:
        try:
            backup_path = f"{file_path}.bak"
            if not Path(backup_path).exists():
                Path(file_path).rename(backup_path)

            with open(file_path, 'w', encoding='utf-8') as f:
                if file_path.endswith(".c") or file_path.endswith(".h"):
                    content = content + "// modified"
                f.write(content)
            logger.info(f"Updated {file_path}")
            return True
        except Exception as e:
            logger.error(f"Error writing {file_path}: {str(e)}")
            return False

    return False


def main():
    try:
        with open('./tool/config.yaml', 'r') as f:
            config = yaml.safe_load(f)
    except Exception as e:
        print(f"Error loading config: {str(e)}")
        return
    logger = init_logger(config)
    logger.info(f"current path: {os.getcwd()}")

    state_file = "./tool/state.txt"
    executed_ops = set()

    if Path(state_file).exists():
        with open(state_file, 'w') as f:
            pass
        with open(state_file, 'r') as f:
            executed_ops = set(line.strip() for line in f)

    new_ops = set()

    for file_spec in config["files"]:
        file_path = file_spec["path"]
        operations = file_spec["operations"]

        if process_file(file_path, operations, executed_ops):
            for op in operations:
                op_id = generate_op_id(file_path, op)
                if op_id not in executed_ops:
                    new_ops.add(op_id)

    if new_ops:
        executed_ops.update(new_ops)
        with open(state_file, 'w+') as f:
            for op_id in executed_ops:
                f.write(f"{op_id}\n")
        logger.info(f"Updated state with {len(new_ops)} new operations")

    logger.info("Processing completed")


if __name__ == "__main__":
    main()
    from img_convert.c_img_convert import process
    process()
