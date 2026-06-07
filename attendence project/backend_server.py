#!/usr/bin/env python3
"""
Student Tracking & Attendance System - Backend Server
Flask-based REST API server with WebSocket support for real-time tracking
"""

from flask import Flask, jsonify, request, render_template
from flask_cors import CORS
from flask_socketio import SocketIO, emit, join_room, leave_room
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime, timedelta
import json
import base64
import os
from functools import wraps

# ==================== Configuration ====================
app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key-here'
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///attendance_system.db'
app.config['JSON_SORT_KEYS'] = False

CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*")
db = SQLAlchemy(app)

# ==================== Database Models ====================

class Student(db.Model):
    """Student model"""
    __tablename__ = 'students'
    
    id = db.Column(db.Integer, primary_key=True)
    student_id = db.Column(db.String(20), unique=True, nullable=False)
    name = db.Column(db.String(100), nullable=False)
    email = db.Column(db.String(100))
    phone = db.Column(db.String(15))
    school_id = db.Column(db.String(20), nullable=False)
    device_id = db.Column(db.String(20))
    face_encoding = db.Column(db.LargeBinary)  # Stored face embedding
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'student_id': self.student_id,
            'name': self.name,
            'email': self.email,
            'school_id': self.school_id
        }


class Attendance(db.Model):
    """Attendance record model"""
    __tablename__ = 'attendance'
    
    id = db.Column(db.Integer, primary_key=True)
    student_id = db.Column(db.String(20), db.ForeignKey('students.student_id'))
    date = db.Column(db.Date, nullable=False)
    check_in_time = db.Column(db.DateTime)
    check_out_time = db.Column(db.DateTime)
    method = db.Column(db.String(50))  # 'face_recognition', 'manual', etc.
    status = db.Column(db.String(20))  # 'present', 'absent', 'late'
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'student_id': self.student_id,
            'date': self.date.isoformat(),
            'check_in_time': self.check_in_time.isoformat() if self.check_in_time else None,
            'check_out_time': self.check_out_time.isoformat() if self.check_out_time else None,
            'method': self.method,
            'status': self.status
        }


class LocationTracking(db.Model):
    """Real-time location tracking data"""
    __tablename__ = 'location_tracking'
    
    id = db.Column(db.Integer, primary_key=True)
    student_id = db.Column(db.String(20), db.ForeignKey('students.student_id'))
    latitude = db.Column(db.Float, nullable=False)
    longitude = db.Column(db.Float, nullable=False)
    accuracy = db.Column(db.Float)
    speed = db.Column(db.Float)  # km/h
    device_id = db.Column(db.String(20))
    battery_level = db.Column(db.Float)
    signal_strength = db.Column(db.Integer)  # dBm
    timestamp = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'student_id': self.student_id,
            'latitude': self.latitude,
            'longitude': self.longitude,
            'accuracy': self.accuracy,
            'speed': self.speed,
            'battery_level': self.battery_level,
            'timestamp': self.timestamp.isoformat()
        }


class Alert(db.Model):
    """Safety alerts"""
    __tablename__ = 'alerts'
    
    id = db.Column(db.Integer, primary_key=True)
    student_id = db.Column(db.String(20), db.ForeignKey('students.student_id'))
    alert_type = db.Column(db.String(50))  # 'geofence', 'unusual_route', 'extended_stay'
    message = db.Column(db.Text)
    severity = db.Column(db.String(20))  # 'low', 'medium', 'high', 'critical'
    acknowledged = db.Column(db.Boolean, default=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'student_id': self.student_id,
            'alert_type': self.alert_type,
            'message': self.message,
            'severity': self.severity,
            'acknowledged': self.acknowledged,
            'created_at': self.created_at.isoformat()
        }


# ==================== Authentication ====================

def token_required(f):
    """Decorator to require authentication token"""
    @wraps(f)
    def decorated_function(*args, **kwargs):
        token = request.headers.get('Authorization')
        
        if not token:
            return jsonify({'message': 'Token missing'}), 401
        
        try:
            # Validate token (simplified for demo)
            if not token.startswith('Bearer '):
                return jsonify({'message': 'Invalid token format'}), 401
            
            token_value = token[7:]  # Remove 'Bearer ' prefix
            
        except Exception as e:
            return jsonify({'message': f'Token validation error: {str(e)}'}), 401
        
        return f(*args, **kwargs)
    
    return decorated_function


# ==================== API Endpoints ====================

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.utcnow().isoformat(),
        'version': '1.0.0'
    })


@app.route('/api/students', methods=['GET'])
@token_required
def get_students():
    """Get all students"""
    students = Student.query.all()
    return jsonify([student.to_dict() for student in students])


@app.route('/api/students/<student_id>', methods=['GET'])
@token_required
def get_student(student_id):
    """Get specific student details"""
    student = Student.query.filter_by(student_id=student_id).first()
    
    if not student:
        return jsonify({'message': 'Student not found'}), 404
    
    return jsonify(student.to_dict())


@app.route('/api/students', methods=['POST'])
@token_required
def create_student():
    """Create new student record"""
    data = request.get_json()
    
    # Validate required fields
    required_fields = ['student_id', 'name', 'school_id']
    if not all(field in data for field in required_fields):
        return jsonify({'message': 'Missing required fields'}), 400
    
    # Check if student already exists
    if Student.query.filter_by(student_id=data['student_id']).first():
        return jsonify({'message': 'Student already exists'}), 409
    
    student = Student(
        student_id=data['student_id'],
        name=data['name'],
        email=data.get('email'),
        phone=data.get('phone'),
        school_id=data['school_id'],
        device_id=data.get('device_id')
    )
    
    db.session.add(student)
    db.session.commit()
    
    return jsonify(student.to_dict()), 201


@app.route('/api/attendance/submit', methods=['POST'])
def submit_attendance():
    """Submit attendance data from IoT device"""
    data = request.get_json()
    
    print(f"[API] Received attendance submission: {data}")
    
    # Extract data
    student_id = data.get('student_id')
    device_id = data.get('device_id')
    timestamp = datetime.fromtimestamp(data.get('timestamp', 0))
    gps_data = data.get('gps', {})
    face_data = data.get('face_recognition', {})
    
    # Store location data
    location = LocationTracking(
        student_id=student_id,
        latitude=gps_data.get('latitude'),
        longitude=gps_data.get('longitude'),
        accuracy=gps_data.get('accuracy'),
        speed=gps_data.get('speed'),
        device_id=device_id,
        timestamp=timestamp
    )
    db.session.add(location)
    
    # Store or update attendance
    today = datetime.utcnow().date()
    attendance = Attendance.query.filter_by(
        student_id=student_id,
        date=today
    ).first()
    
    if not attendance:
        attendance = Attendance(
            student_id=student_id,
            date=today,
            method=data.get('method', 'face_recognition'),
            status=face_data.get('status', 'present')
        )
        db.session.add(attendance)
    
    attendance.check_in_time = timestamp
    attendance.status = face_data.get('status', 'present')
    
    db.session.commit()
    
    # Broadcast location update via WebSocket
    socketio.emit('location_update', {
        'student_id': student_id,
        'latitude': gps_data.get('latitude'),
        'longitude': gps_data.get('longitude'),
        'speed': gps_data.get('speed'),
        'timestamp': timestamp.isoformat()
    }, broadcast=True)
    
    return jsonify({
        'message': 'Attendance submitted successfully',
        'id': attendance.id
    }), 201


@app.route('/api/attendance/<date>', methods=['GET'])
@token_required
def get_attendance(date):
    """Get attendance records for a specific date"""
    try:
        attendance_date = datetime.strptime(date, '%Y-%m-%d').date()
    except ValueError:
        return jsonify({'message': 'Invalid date format'}), 400
    
    records = Attendance.query.filter_by(date=attendance_date).all()
    return jsonify([record.to_dict() for record in records])


@app.route('/api/attendance/student/<student_id>', methods=['GET'])
@token_required
def get_student_attendance(student_id):
    """Get attendance history for a student"""
    days = request.args.get('days', 30, type=int)
    start_date = datetime.utcnow().date() - timedelta(days=days)
    
    records = Attendance.query.filter(
        Attendance.student_id == student_id,
        Attendance.date >= start_date
    ).all()
    
    return jsonify([record.to_dict() for record in records])


@app.route('/api/location/latest/<student_id>', methods=['GET'])
@token_required
def get_latest_location(student_id):
    """Get latest location of a student"""
    location = LocationTracking.query.filter_by(
        student_id=student_id
    ).order_by(LocationTracking.timestamp.desc()).first()
    
    if not location:
        return jsonify({'message': 'No location data found'}), 404
    
    return jsonify(location.to_dict())


@app.route('/api/location/history/<student_id>', methods=['GET'])
@token_required
def get_location_history(student_id):
    """Get location history for a student"""
    hours = request.args.get('hours', 24, type=int)
    start_time = datetime.utcnow() - timedelta(hours=hours)
    
    locations = LocationTracking.query.filter(
        LocationTracking.student_id == student_id,
        LocationTracking.timestamp >= start_time
    ).order_by(LocationTracking.timestamp.desc()).all()
    
    return jsonify([location.to_dict() for location in locations])


@app.route('/api/alerts', methods=['GET'])
@token_required
def get_alerts():
    """Get recent alerts"""
    hours = request.args.get('hours', 24, type=int)
    start_time = datetime.utcnow() - timedelta(hours=hours)
    
    alerts = Alert.query.filter(
        Alert.created_at >= start_time
    ).order_by(Alert.created_at.desc()).all()
    
    return jsonify([alert.to_dict() for alert in alerts])


@app.route('/api/alerts', methods=['POST'])
def create_alert():
    """Create a new alert"""
    data = request.get_json()
    
    alert = Alert(
        student_id=data.get('student_id'),
        alert_type=data.get('alert_type'),
        message=data.get('message'),
        severity=data.get('severity', 'medium')
    )
    
    db.session.add(alert)
    db.session.commit()
    
    # Broadcast alert via WebSocket
    socketio.emit('alert', alert.to_dict(), broadcast=True)
    
    return jsonify(alert.to_dict()), 201


@app.route('/api/alerts/<int:alert_id>/acknowledge', methods=['PUT'])
@token_required
def acknowledge_alert(alert_id):
    """Acknowledge an alert"""
    alert = Alert.query.get(alert_id)
    
    if not alert:
        return jsonify({'message': 'Alert not found'}), 404
    
    alert.acknowledged = True
    db.session.commit()
    
    return jsonify(alert.to_dict())


# ==================== WebSocket Events ====================

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    print(f"[WebSocket] Client connected: {request.sid}")
    emit('response', {'message': 'Connected to server'})


@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    print(f"[WebSocket] Client disconnected: {request.sid}")


@socketio.on('subscribe')
def handle_subscribe(data):
    """Subscribe to student tracking updates"""
    student_id = data.get('student_id')
    join_room(f'student_{student_id}')
    print(f"[WebSocket] Subscribed to student: {student_id}")
    emit('response', {'message': f'Subscribed to {student_id}'})


@socketio.on('unsubscribe')
def handle_unsubscribe(data):
    """Unsubscribe from student tracking updates"""
    student_id = data.get('student_id')
    leave_room(f'student_{student_id}')
    print(f"[WebSocket] Unsubscribed from student: {student_id}")
    emit('response', {'message': f'Unsubscribed from {student_id}'})


# ==================== Error Handlers ====================

@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors"""
    return jsonify({'message': 'Resource not found'}), 404


@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors"""
    db.session.rollback()
    return jsonify({'message': 'Internal server error'}), 500


# ==================== Database Initialization ====================

def init_database():
    """Initialize database with sample data"""
    with app.app_context():
        db.create_all()
        
        # Add sample students if not exists
        if Student.query.count() == 0:
            students_data = [
                {'student_id': 'STU_001', 'name': 'Ravi Kumar', 'school_id': 'SCHOOL_001', 'email': 'ravi@school.com'},
                {'student_id': 'STU_002', 'name': 'Emma Wilson', 'school_id': 'SCHOOL_001', 'email': 'emma@school.com'},
                {'student_id': 'STU_003', 'name': 'Alex Johnson', 'school_id': 'SCHOOL_001', 'email': 'alex@school.com'},
            ]
            
            for data in students_data:
                student = Student(**data)
                db.session.add(student)
            
            db.session.commit()
            print("[DB] Sample students added")


# ==================== Main ====================

if __name__ == '__main__':
    print("=" * 60)
    print("  Student Tracking Backend Server v1.0")
    print("=" * 60)
    
    # Initialize database
    init_database()
    
    # Start server
    print("\n[SERVER] Starting Flask server...")
    print("[SERVER] API Base URL: http://localhost:5000/api")
    print("[SERVER] WebSocket URL: ws://localhost:5000/socket.io")
    
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
