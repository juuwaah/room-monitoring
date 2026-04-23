import os
from datetime import datetime, timezone
from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# In-memory status store
current_status = {
    "name": "Staff",
    "status": "unknown",
    "minutes": 0,
    "updated_at": None,
}


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/status", methods=["GET"])
def get_status():
    return jsonify(current_status)


@app.route("/api/status", methods=["POST"])
def update_status():
    data = request.get_json(force=True)

    status = data.get("status")
    minutes = data.get("minutes")

    if status not in ("in_room", "out"):
        return jsonify({"error": "status must be 'in_room' or 'out'"}), 400
    if not isinstance(minutes, (int, float)) or minutes < 0:
        return jsonify({"error": "minutes must be a non-negative number"}), 400

    current_status["status"] = status
    current_status["minutes"] = int(minutes)
    current_status["updated_at"] = datetime.now(timezone.utc).isoformat()

    return jsonify({"ok": True, "status": current_status})


if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5000))
    app.run(host="0.0.0.0", port=port, debug=True)
