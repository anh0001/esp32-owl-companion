import React, { useState, useEffect, useCallback } from 'react';
import { LineChart, Line, BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ReferenceLine, ResponsiveContainer, AreaChart, Area } from 'recharts';
import './GardenWatchClient.css'; // added CSS import

const GardenWatchClient = () => {
  // State for the different data types
  const [gardenActivityData, setGardenActivityData] = useState([]);
  const [dailyPatterns, setDailyPatterns] = useState({ healthy: [], current: [] });
  const [weeklyTaskData, setWeeklyTaskData] = useState({ healthy: [], current: [] });
  const [deviationData, setDeviationData] = useState([]);
  const [reminderData, setReminderData] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [owlIp, setOwlIp] = useState('192.168.1.100'); // Default IP
  
  // Wrap sample data generation functions with useCallback
  const generateGardenActivityData = useCallback(() => {
    const data = [];
    for (let day = 1; day <= 28; day++) {
      const week = Math.ceil(day / 7);
      const isHealthyWeek = week <= 2;
      
      // Baseline activity (minutes in garden)
      const baselineActivity = 80;
      
      // Calculate actual activity
      let actualActivity;
      if (isHealthyWeek) {
        // Weeks 1-2: normal pattern with minor variations
        actualActivity = baselineActivity + (Math.random() * 20 - 10);
      } else {
        // Weeks 3-4: declining pattern
        const declineFactor = 0.8 * ((day - 14) / 14); // Progressive decline
        actualActivity = baselineActivity * (1 - declineFactor) + (Math.random() * 10 - 5);
      }
      
      // Calculate deviation score
      const standardDeviation = 10;
      const deviationScore = Math.abs(actualActivity - baselineActivity) / standardDeviation;
      
      // Determine if alert should be triggered
      const alertThreshold = 1.5;
      const alert = deviationScore >= alertThreshold;
      
      data.push({
        day: `Day ${day}`,
        actualActivity: Math.max(0, Math.round(actualActivity)),
        baselineActivity,
        deviationScore: Math.round(deviationScore * 100) / 100,
        alert
      });
    }
    return data;
  }, []);

  const generateDailyActivityData = useCallback((isHealthy) => {
    const hours = Array.from({ length: 24 }, (_, i) => i);
    
    return hours.map(hour => {
      // Normal gardening pattern: morning watering (6-8), midday plant care (10-12), 
      // evening maintenance (17-19)
      let activity = 0;
      
      if (hour >= 6 && hour <= 8) activity = 80; // morning watering routine
      else if (hour >= 10 && hour <= 12) activity = 65; // midday plant checks
      else if (hour >= 17 && hour <= 19) activity = 75; // evening garden maintenance
      else if (hour >= 22 || hour <= 5) activity = 5;  // nighttime (minimal activity)
      else activity = 20; // occasional checks throughout day
      
      // For unhealthy pattern: declining engagement with garden
      if (!isHealthy) {
        if (hour >= 6 && hour <= 8) activity *= 0.4;     // reduced morning gardening
        else if (hour >= 10 && hour <= 12) activity *= 0.3; // minimal midday gardening
        else if (hour >= 17 && hour <= 19) activity *= 0.2; // almost no evening gardening
        else if (hour >= 22 || hour <= 5) activity = 20;    // unusual nighttime activity
      }
      
      // Add small random variation
      activity = Math.round(activity * (0.9 + Math.random() * 0.2));
      
      return {
        hour: `${hour}:00`,
        activity,
        expectedActivity: isHealthy ? 0 : 
          (hour >= 6 && hour <= 8) ? 80 : 
          (hour >= 10 && hour <= 12) ? 65 :
          (hour >= 17 && hour <= 19) ? 75 : 20
      };
    });
  }, []);

  const generateWeeklyGardeningData = useCallback((isHealthy) => {
    const weekdays = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
    
    return weekdays.map(day => {
      // Healthy pattern shows regular engagement with different gardening tasks
      const tasksCompleted = isHealthy ? 
        Math.floor(3 + Math.random() * 3) : // 3-5 tasks when healthy
        Math.floor(Math.random() * 2);      // 0-1 tasks when unhealthy
      
      // Time spent in garden (minutes)
      const gardenTime = isHealthy ?
        Math.floor(40 + Math.random() * 30) : // 40-70 minutes when healthy
        Math.floor(Math.random() * 20);      // 0-20 minutes when unhealthy
      
      return {
        day,
        tasksCompleted,
        gardenTime
      };
    });
  }, []);

  const generateReminderData = useCallback(() => {
    const weeks = ['Week 1', 'Week 2', 'Week 3', 'Week 4'];
    return weeks.map(week => {
      const weekNum = parseInt(week.split(' ')[1]);
      const isEarlyWeek = weekNum <= 2;
      
      // Response time increases in later weeks
      const responseTime = isEarlyWeek ? 
        10 + Math.random() * 15 : // 10-25 minutes in early weeks
        25 + Math.random() * 35;  // 25-60 minutes in later weeks
      
      return {
        week,
        responseTime: Math.round(responseTime),
        alert: responseTime > 30
      };
    });
  }, []);

  // Refactor fetchData with useCallback and include data generator dependencies
  const fetchData = useCallback(async () => {
    setLoading(true);
    setError(null);
    
    try {
      // For demo purposes, we'll use sample data to avoid requiring the actual hardware
      // In a real implementation, you would uncomment these and use the actual API endpoints
      
      // const activityResponse = await fetch(`http://${owlIp}/api/data/activity`);
      // const activityData = await activityResponse.json();
      // setGardenActivityData(activityData);
      
      // const patternsResponse = await fetch(`http://${owlIp}/api/data/daily-patterns`);
      // const patternsData = await patternsResponse.json();
      // setDailyPatterns(patternsData);
      
      // const weeklyResponse = await fetch(`http://${owlIp}/api/data/weekly-tasks`);
      // const weeklyData = await weeklyResponse.json();
      // setWeeklyTaskData(weeklyData);
      
      // const deviationResponse = await fetch(`http://${owlIp}/api/data/deviation`);
      // const deviationData = await deviationResponse.json();
      // setDeviationData(deviationData);
      
      // const reminderResponse = await fetch(`http://${owlIp}/api/data/reminders`);
      // const reminderData = await reminderResponse.json();
      // setReminderData(reminderData);
      
      // For demo, we'll generate sample data similar to what the API would return
      const activityData = generateGardenActivityData();
      setGardenActivityData(activityData);
      setDailyPatterns({
        healthy: generateDailyActivityData(true),
        current: generateDailyActivityData(false)
      });
      setWeeklyTaskData({
        healthy: generateWeeklyGardeningData(true),
        current: generateWeeklyGardeningData(false)
      });
      setDeviationData(activityData.map(item => ({
        day: item.day,
        deviationScore: item.deviationScore,
        alert: item.alert
      })));
      setReminderData(generateReminderData());
      
      setLoading(false);
    } catch (err) {
      console.error('Error fetching data:', err);
      setError('Failed to fetch data from the Garden Watch system. Please check the connection and try again.');
      setLoading(false);
    }
  }, [generateGardenActivityData, generateDailyActivityData, generateWeeklyGardeningData, generateReminderData, owlIp]);
  
  // Fetch data on component mount and when owl IP changes
  useEffect(() => {
    fetchData();
    // In a real app, you might want to set up a refresh interval
    // const interval = setInterval(fetchData, 60000); // Refresh every minute
    // return () => clearInterval(interval);
  }, [fetchData]);
  
  // Handle form submission for IP address
  const handleSubmit = (e) => {
    e.preventDefault();
    fetchData();
  };

  if (loading) {
    return (
      <div className="loading-container">
        <div className="loading-content">
          <div className="loading-spinner"></div>
          <p>Loading data from Garden Watch system...</p>
        </div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="alert-box" role="alert">
        <strong className="bold-text">Error!</strong>
        <span> {error}</span>
      </div>
    );
  }

  return (
    <div className="garden-watch-container">
      <div className="header-container">
        <div>
          <h2 className="title">Garden Watch: Front Garden Activity Monitoring</h2>
          <p>This visualization demonstrates how the Garden Watch system uses pattern recognition to detect health concerns through changes in gardening habits.</p>
        </div>
        <form onSubmit={handleSubmit} className="form-container">
          <label className="small-text bold-text">Owl IP:</label>
          <input 
            type="text" 
            value={owlIp} 
            onChange={e => setOwlIp(e.target.value)}
            className="input-field"
            placeholder="192.168.1.100"
          />
          <button 
            type="submit" 
            className="button"
          >
            Connect
          </button>
        </form>
      </div>

      {/* Activity and Deviation Chart */}
      <div className="card">
        <h3 className="subtitle">Front Garden Activity Monitoring</h3>
        <p>The system establishes baseline activity patterns and measures deviations that may indicate health concerns.</p>
        
        <div className="chart-container">
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={gardenActivityData}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="day" tick={{fontSize: 10}} interval={3} />
              <YAxis label={{ value: 'Minutes in Garden', angle: -90, position: 'insideLeft', offset: 5, style: { textAnchor: 'middle' } }} />
              <Tooltip />
              <Legend />
              <Line type="monotone" dataKey="actualActivity" stroke="#4CAF50" name="Actual Garden Time" strokeWidth={2} />
              <Line type="monotone" dataKey="baselineActivity" stroke="#2196F3" name="Expected Baseline" strokeDasharray="5 5" />
              <ReferenceLine y={40} stroke="#FF9800" strokeDasharray="3 3" label="Minimum Healthy Activity" />
            </LineChart>
          </ResponsiveContainer>
        </div>
        
        <div className="text-box">
          <h4 className="bold-text">Mathematical Model Correlation:</h4>
          <p>This chart implements Equation (5) from the paper, establishing a baseline activity pattern for garden engagement:</p>
          <p className="code-block">B(t,w) = (1/N)∑A(d+7(w-1))(t)</p>
          <p>Where:</p>
          <ul className="list">
            <li>B(t,w) is the baseline activity (blue dashed line)</li>
            <li>A(d)(t) is the actual activity on day d during time t (green line)</li>
          </ul>
          <p className="mt-4">The declining pattern in weeks 3-4 triggers alerts according to Equation (8):</p>
          <p className="code-block">Alert(d) = 1 if ∑S(d)(t) ≥ θA</p>
        </div>
      </div>

      {/* Daily Garden Activity Patterns */}
      <div className="card">
        <h3 className="subtitle">Daily Garden Activity Patterns</h3>
        <p>The system establishes baseline patterns of garden engagement throughout the day, detecting changes that may indicate health concerns.</p>
        
        <div className="grid-container">
          <div>
            <h4 className="subtitle">Healthy Gardening Pattern</h4>
            <div className="chart-container">
              <ResponsiveContainer width="100%" height="100%">
                <AreaChart data={dailyPatterns.healthy}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="hour" />
                  <YAxis label={{ value: 'Activity Level (%)', angle: -90, position: 'insideLeft', offset: 5, style: { textAnchor: 'middle' } }} />
                  <Tooltip />
                  <Area type="monotone" dataKey="activity" stroke="#4caf50" fill="#4caf50" fillOpacity={0.3} name="Garden Activity" />
                  <ReferenceLine y={30} stroke="#ff7300" strokeDasharray="3 3" label="Threshold" />
                </AreaChart>
              </ResponsiveContainer>
            </div>
            <p className="text-box">Regular garden engagement with morning watering (6-8am), midday plant checks (10am-12pm), and evening garden maintenance (5-7pm).</p>
          </div>
          
          <div>
            <h4 className="subtitle">Concerning Gardening Pattern</h4>
            <div className="chart-container">
              <ResponsiveContainer width="100%" height="100%">
                <AreaChart data={dailyPatterns.current}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="hour" />
                  <YAxis />
                  <Tooltip />
                  <ReferenceLine y={30} stroke="#ff7300" strokeDasharray="3 3" label="Threshold" />
                  <Area type="monotone" dataKey="activity" stroke="#f44336" fill="#f44336" fillOpacity={0.3} name="Actual Activity" />
                  <Line type="monotone" dataKey="expectedActivity" stroke="#2196f3" strokeDasharray="5 5" name="Expected Activity" />
                </AreaChart>
              </ResponsiveContainer>
            </div>
            <p className="text-box">Significant reduction in garden engagement across all time periods, with unusual nighttime activity. This pattern may indicate physical limitations, cognitive changes, or sleep disturbances.</p>
          </div>
        </div>
        <div className="info-box">
          <h4 className="bold-text">Detection Algorithm Correlation:</h4>
          <p>These patterns implement equations (1-4) from the paper, which detect human presence in the garden:</p>
          <p className="code-block">H(i,j) = 1 if 22°C ≤ T(i,j) ≤ 31°C</p>
          <p className="code-block">P = 1 if ∑∑C(i,j) ≥ θP</p>
          <p>The system builds profiles of typical garden usage times and can detect unusual deviations like reduced morning activity or increased nighttime presence.</p>
        </div>
      </div>

      {/* Weekly Garden Task Completion */}
      <div className="card">
        <h3 className="subtitle">Weekly Garden Task Completion</h3>
        <p>Garden Watch monitors consistency in completing routine gardening tasks, which can indicate cognitive and physical well-being.</p>
        
        <div className="grid-container">
          <div>
            <h4 className="subtitle">Healthy Gardening Routine</h4>
            <div className="chart-container">
              <ResponsiveContainer width="100%" height="100%">
                <BarChart data={weeklyTaskData.healthy}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="day" />
                  <YAxis yAxisId="left" orientation="left" label={{ value: 'Tasks Completed', angle: -90, position: 'insideLeft', offset: 5, style: { textAnchor: 'middle' } }} />
                  <YAxis yAxisId="right" orientation="right" label={{ value: 'Garden Time (min)', angle: 90, position: 'insideRight', offset: 10, style: { textAnchor: 'middle' } }} />
                  <Tooltip />
                  <Legend />
                  <Bar yAxisId="left" dataKey="tasksCompleted" fill="#4caf50" name="Tasks Completed" />
                  <Line yAxisId="right" type="monotone" dataKey="gardenTime" stroke="#2196f3" name="Time in Garden (min)" />
                </BarChart>
              </ResponsiveContainer>
            </div>
            <div className="text-box">
              <p>Consistent engagement with gardening tasks throughout the week:</p>
              <ul className="list">
                <li>Regular watering, pruning, and plant care</li>
                <li>Consistent time spent in garden (40-70 minutes daily)</li>
                <li>Multiple tasks completed each day</li>
              </ul>
            </div>
          </div>
          
          <div>
            <h4 className="subtitle">Concerning Gardening Routine</h4>
            <div className="chart-container">
              <ResponsiveContainer width="100%" height="100%">
                <BarChart data={weeklyTaskData.current}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="day" />
                  <YAxis yAxisId="left" orientation="left" />
                  <YAxis yAxisId="right" orientation="right" />
                  <Tooltip />
                  <Legend />
                  <Bar yAxisId="left" dataKey="tasksCompleted" fill="#f44336" name="Tasks Completed" />
                  <Line yAxisId="right" type="monotone" dataKey="gardenTime" stroke="#2196f3" name="Time in Garden (min)" />
                </BarChart>
              </ResponsiveContainer>
            </div>
            <div className="text-box">
              <p>Significant reduction in gardening engagement:</p>
              <ul className="list">
                <li>Minimal or skipped watering routines</li>
                <li>Very little time spent in garden (less than 20 minutes)</li>
                <li>Few or no gardening tasks completed</li>
              </ul>
            </div>
          </div>
        </div>
        <div className="info-box">
          <h4 className="bold-text">Correlation to Pattern Recognition Algorithm:</h4>
          <p>This visualization demonstrates how the baseline calculation from equation (5) is applied to different types of garden activities:</p>
          <p className="code-block">B(t,w) = (1/N)∑A(d+7(w-1))(t)</p>
          <p>Significant deviations in task completion and garden time can trigger wellness checks through equation (11).</p>
        </div>
      </div>

      {/* Deviation Score Chart */}
      <div className="card">
        <h3 className="subtitle">Deviation Score Analysis</h3>
        <p>This chart shows how the system quantifies deviations from expected gardening behavior.</p>
        
        <div className="chart-container">
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={deviationData}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="day" tick={{fontSize: 10}} interval={3} />
              <YAxis label={{ value: 'Deviation Score', angle: -90, position: 'insideLeft', offset: 5, style: { textAnchor: 'middle' } }} />
              <Tooltip />
              <Legend />
              <Line type="monotone" dataKey="deviationScore" stroke="#F44336" name="Deviation Score S(d)(t)" />
              <ReferenceLine y={1.5} stroke="#D32F2F" strokeDasharray="3 3" label="Alert Threshold θA" />
            </LineChart>
          </ResponsiveContainer>
        </div>
        
        <div className="text-box">
          <h4 className="bold-text">Deviation Score Calculation:</h4>
          <p>The system calculates deviation scores using Equation (7) from the paper:</p>
          <p className="code-block">S(d)(t) = |A(d)(t)-B(t,w)|/(σ(t,w)+ε)</p>
          <p>Where:</p>
          <ul className="list">
            <li>S(d)(t) is the deviation score (red line)</li>
            <li>A(d)(t) is the actual activity</li>
            <li>B(t,w) is the baseline activity</li>
            <li>σ(t,w) is the standard deviation of activity</li>
            <li>ε is a small value to prevent division by zero</li>
          </ul>
          <p className="italic-text">Scores above the alert threshold trigger wellness checks, as they may indicate health issues.</p>
        </div>
      </div>

      {/* Reminder Response Chart */}
      <div className="card">
        <h3 className="subtitle">Response to Garden Care Reminders</h3>
        <p>The Garden Watch owl provides plant care reminders. Response times provide insights into cognitive and physical function.</p>
        
        <div className="chart-container">
          <ResponsiveContainer width="100%" height="100%">
            <BarChart data={reminderData}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="week" />
              <YAxis label={{ value: 'Response Time (min)', angle: -90, position: 'insideLeft', offset: 5, style: { textAnchor: 'middle' } }} />
              <Tooltip />
              <Legend />
              <Bar dataKey="responseTime" fill="#2196F3" name="Response Time to Reminders" />
              <ReferenceLine y={30} stroke="#F44336" strokeDasharray="3 3" label="Alert Threshold" />
            </BarChart>
          </ResponsiveContainer>
        </div>
        
        <div className="text-box">
          <h4 className="bold-text">User Interaction Model:</h4>
          <p>This chart implements the user interaction algorithms from Equations (9-11) in the paper:</p>
          <p className="code-block">R(plant)(d,t) = 1 if (d mod f(plant)) = 0 and t = t(plant)</p>
          <p className="code-block">M(t) = M(wellness) if response time > threshold</p>
          <p>The increasing response times in weeks 3-4 trigger wellness interaction mode (M(wellness)), prompting the owl to check on the elderly user.</p>
        </div>
      </div>

      {/* Framework Overview */}
      <div className="card">
        <h3 className="subtitle">Garden Watch Monitoring Framework</h3>
        
        <div className="grid-container">
          <div>
            <h4 className="subtitle">Activity Monitoring</h4>
            <p>The system uses passive infrared sensing to detect presence in the garden:</p>
            <div className="code-block">
              H(i,j) = 1 if 22°C ≤ T(i,j) ≤ 31°C<br/>
              P = 1 if ∑∑C(i,j) ≥ θP
            </div>
            <p>This allows tracking garden engagement while preserving privacy.</p>
          </div>
          
          <div>
            <h4 className="subtitle">Pattern Recognition</h4>
            <p>The system establishes personalized baselines and detects deviations:</p>
            <div className="code-block">
              B(t,w) = (1/N)∑A(d+7(w-1))(t)<br/>
              S(d)(t) = |A(d)(t)-B(t,w)|/(σ(t,w)+ε)
            </div>
            <p>These algorithms can detect changes up to 11 days before adverse health events.</p>
          </div>
          
          <div>
            <h4 className="subtitle">Health Insights from Gardening</h4>
            <ul className="list">
              <li>Reduced garden time may indicate physical limitations</li>
              <li>Missed plant care tasks may suggest memory issues</li>
              <li>Irregular patterns may signal cognitive changes</li>
              <li>Slower response to reminders could indicate reduced mobility</li>
            </ul>
          </div>
          
          <div>
            <h4 className="subtitle">Non-Intrusive Design</h4>
            <p>The owl-shaped design provides several benefits:</p>
            <ul className="list">
              <li>Appears as garden decor rather than medical equipment</li>
              <li>Reduces stigma associated with monitoring technology</li>
              <li>Integrates naturally into therapeutic gardening environment</li>
              <li>Provides non-clinical interactions through nodding, sounds, and vibration</li>
            </ul>
          </div>
        </div>
        
        <div className="info-box">
          <p className="italic-text">The Garden Watch framework addresses both technical monitoring requirements and the social-psychological factors affecting acceptance. This implementation preserves elderly dignity and autonomy while providing valuable health insights that can significantly improve care outcomes.</p>
        </div>
      </div>
    </div>
  );
};

export default GardenWatchClient;