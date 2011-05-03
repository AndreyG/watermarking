import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.data.xy.DefaultXYDataset;
import org.jfree.data.xy.XYDataset;

import javax.swing.*;
import java.awt.*;
import java.util.ArrayList;
import java.util.List;

public class GraphicPanel extends JPanel {

    public GraphicPanel(final ArrayList<ArrayList<Double>> arguments, final ArrayList<ArrayList<Double>> values,
                        final List<String> titles, String argumentName, String functionName) {

        this.setLayout(new BoxLayout(this, BoxLayout.PAGE_AXIS));

        final ChartPanel chartPanel = new ChartPanel(createChart(createDataset(arguments, values, titles), argumentName, functionName), false);
        chartPanel.setPreferredSize(new Dimension(500, 270));
        this.add(chartPanel);
    }

	private double[] listToArray(List<Double> list) {
		double[] res = new double[list.size()];
		for (int i = 0; i != res.length; ++i)
			res[i] = list.get(i);
		return res;
	}

    private XYDataset createDataset(ArrayList<ArrayList<Double>> arguments, ArrayList<ArrayList<Double>> values, List<String> titles) {
        DefaultXYDataset dataset = new DefaultXYDataset();

        for (int j = 0; j < titles.size(); j++) {
			double[] x = listToArray(arguments.get(j));
			double[] y = listToArray(values.get(j));

            double[][] data = new double[][]{x, y};
            dataset.addSeries(titles.get(j), data);
        }
        return dataset;
    }

    private static JFreeChart createChart(XYDataset dataset, String argumentName, String functionName) {

        // create the chart...
        JFreeChart chart = ChartFactory.createXYLineChart(
                functionName,       // chart title
                argumentName,                        // domain axis label
                functionName,                        // range axis label
                dataset,                  // data
                PlotOrientation.VERTICAL, // orientation
                true,                     // include legend
                true,                     // tooltips?
                false                     // URLs?
        );

        // NOW DO SOME OPTIONAL CUSTOMISATION OF THE CHART...

        // set the background color for the chart...
        chart.setBackgroundPaint(Color.white);

        // get a reference to the plot for further customisation...
        XYPlot plot = (XYPlot) chart.getPlot();
        plot.setBackgroundPaint(Color.white);
        plot.setDomainGridlinePaint(Color.GRAY);
        plot.setDomainGridlinesVisible(true);
        plot.setRangeGridlinePaint(Color.lightGray);

        // ******************************************************************
        //  More than 150 demo applications are included with the JFreeChart
        //  Developer Guide...for more information, see:
        //
        //  >   http://www.object-refinery.com/jfreechart/guide.html
        //
        // ******************************************************************

        // set the range axis to display integers only...
        //final NumberAxis rangeAxis = (NumberAxis) plot.getRangeAxis();
        //rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
        ValueAxis axis = plot.getRangeAxis();
        axis.setRange(0, 1.1);

        // disable bar outlines...
        XYItemRenderer renderer = plot.getRenderer();
        //renderer.setDrawBarOutline(false);

        BasicStroke stroke = new BasicStroke(2);
        for (int i = 0; i < 5; i++) {
            renderer.setSeriesStroke(i, stroke);
        }
        // set up gradient paints for series...
        GradientPaint gp[] = {
			new GradientPaint(0.0f, 0.0f, Color.blue,  0.0f, 0.0f, new Color(0, 0, 64)),
			new GradientPaint(0.0f, 0.0f, Color.green, 0.0f, 0.0f, new Color(0, 64, 0)),
			new GradientPaint(0.0f, 0.0f, Color.red,   0.0f, 0.0f, new Color(64, 0, 0)),
			new GradientPaint(0.0f, 0.0f, Color.cyan,  0.0f, 0.0f, Color.cyan),
			new GradientPaint(0.0f, 0.0f, Color.pink,  0.0f, 0.0f, Color.pink)
		};

		for (int i = 0; i != gp.length; ++i)
			renderer.setSeriesPaint(i, gp[i]);

        // OPTIONAL CUSTOMISATION COMPLETED.

        return chart;
    }
}
