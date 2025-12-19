from flask import Flask, jsonify, request, render_template
from flask_cors import CORS
import subprocess
import os

app = Flask(__name__)
CORS(app)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CPP_EXE = os.path.join(BASE_DIR, "myfs.exe") if os.name == 'nt' else "./myfs"

def run_cpp(args):
    if not os.path.exists(CPP_EXE): return "ERROR: Backend Not Found"
    try:
        result = subprocess.run([CPP_EXE] + args, capture_output=True, text=True, cwd=BASE_DIR)
        return result.stdout.strip()
    except Exception as e: return f"ERROR: {str(e)}"

@app.route('/')
def index(): return render_template('index.html')

@app.route('/api/list', methods=['GET'])
def list_files():
    output = run_cpp(["list"])
    files = []
    recovery_msg = None
    if output and "ERROR" not in output and output != "NONE":
        if "WARNING" in output:
            parts = output.split(";")
            recovery_msg = parts[0].replace("_", " ") + ": " + parts[1].replace("_", " ")
            raw_files = parts[2:]
        else:
            raw_files = output.split(";")
        for f in raw_files:
            if "," in f:
                p = f.split(",")
                files.append({"name": p[0], "block": int(p[1]), "size": int(p[2])})
    return jsonify({"files": files, "message": recovery_msg})

@app.route('/api/create', methods=['POST'])
def create():
    d = request.json
    return jsonify({"status": run_cpp(["create", d.get('name', ''), d.get('content', '')])})

@app.route('/api/read', methods=['POST'])
def read():
    d = request.json
    return jsonify({"content": run_cpp(["read", d.get('name', '')])})

@app.route('/api/update', methods=['POST'])
def update():
    d = request.json
    return jsonify({"status": run_cpp(["update", d.get('name', ''), d.get('content', '')])})

@app.route('/api/delete', methods=['POST'])
def delete():
    d = request.json
    return jsonify({"status": run_cpp(["delete", d.get('name', '')])})

@app.route('/api/crash', methods=['POST'])
def crash(): return jsonify({"status": run_cpp(["crash"])})

@app.route('/api/optimize', methods=['POST'])
def optimize(): return jsonify({"status": run_cpp(["optimize"])})

if __name__ == '__main__':
    app.run(port=5000, debug=True)