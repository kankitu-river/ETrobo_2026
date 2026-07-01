import math
import os

# ===== ロボットパラメータ (実機に合わせて調整) =====
WHEEL_RADIUS_MM = 28.0   # 車輪半径 [mm] (直径56mm)
WHEEL_BASE_MM   = 95.5  # 左右車輪間距離 [mm]

LOG_FILE = os.path.join(os.path.dirname(__file__), "START TRACE LOG----sample_c5_log---.txt")
OUT_FILE = os.path.join(os.path.dirname(__file__), "odometry.csv")

def parse_log(filepath):
    data = []
    in_data = False
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line.startswith('time_ms'):
                in_data = True
                continue
            if not in_data:
                continue
            if line.startswith('*') or line.startswith('log_end') or not line:
                break
            parts = line.split(',')
            if len(parts) == 5:
                try:
                    data.append({
                        'time_ms':     int(parts[0]),
                        'left_count':  int(parts[3]),
                        'right_count': int(parts[4])
                    })
                except ValueError:
                    pass
    return data

def compute_odometry(data):
    results = []
    x, y, theta = 0.0, 0.0, 0.0

    for i, row in enumerate(data):
        if i == 0:
            results.append({'time_ms': row['time_ms'], 'x_mm': 0.0, 'y_mm': 0.0, 'theta_deg': 0.0})
            continue

        dl_deg = row['left_count']  - data[i-1]['left_count']
        dr_deg = row['right_count'] - data[i-1]['right_count']

        dl = (dl_deg / 360.0) * 2 * math.pi * WHEEL_RADIUS_MM
        dr = (dr_deg / 360.0) * 2 * math.pi * WHEEL_RADIUS_MM

        d      = (dl + dr) / 2.0
        dtheta = (dr - dl) / WHEEL_BASE_MM

        theta += dtheta
        x += d * math.cos(theta - dtheta / 2.0)
        y += d * math.sin(theta - dtheta / 2.0)

        results.append({
            'time_ms':   row['time_ms'],
            'x_mm':      round(x, 1),
            'y_mm':      round(y, 1),
            'theta_deg': round(math.degrees(theta), 2)
        })

    return results

def main():
    data = parse_log(LOG_FILE)
    print(f"ログ読み込み: {len(data)} 件")

    if not data:
        print("データが読み込めませんでした")
        return

    results = compute_odometry(data)

    with open(OUT_FILE, 'w', encoding='utf-8') as f:
        f.write('time_ms,x_mm,y_mm,theta_deg\n')
        for r in results:
            f.write(f"{r['time_ms']},{r['x_mm']},{r['y_mm']},{r['theta_deg']}\n")

    print(f"結果を保存: {OUT_FILE}")

    last = results[-1]
    print(f"最終位置: x={last['x_mm']:.1f}mm, y={last['y_mm']:.1f}mm, 角度={last['theta_deg']:.1f}度")

    try:
        import matplotlib.pyplot as plt
        xs = [r['x_mm'] for r in results]
        ys = [r['y_mm'] for r in results]
        plt.figure(figsize=(8, 8))
        plt.plot(xs, ys, 'b-o', markersize=3)
        plt.plot(xs[0], ys[0], 'go', markersize=10, label='スタート')
        plt.plot(xs[-1], ys[-1], 'ro', markersize=10, label='ゴール')
        plt.xlabel('x [mm]')
        plt.ylabel('y [mm]')
        plt.title('走行軌跡 (オドメトリ)')
        plt.legend()
        plt.axis('equal')
        plt.grid(True)
        img_file = os.path.join(os.path.dirname(__file__), "trajectory.png")
        plt.savefig(img_file)
        plt.show()
        print(f"軌跡グラフを保存: {img_file}")
    except ImportError:
        print("matplotlib がないのでグラフは省略 (pip install matplotlib で追加可能)")

if __name__ == '__main__':
    main()
