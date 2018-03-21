function output = FixEditDate( obj, aveCnkPath, dataPath )

	aveObj = dir(aveCnkPath);
    dateAve = datetime(aveObj(1).date, 'Locale',  get(0, 'Language'));

    dataObj = dir(dataPath);
    dateData = datetime(dataObj(1).date, 'Locale',  get(0, 'Language'));

    % Means we  created the data in the future? nopey dopey.
    if (dateData > dateAve)
    	system(['touch -r ', dataPath, ' ', aveCnkPath]);
    end
end